#!/usr/bin/env python3
"""
Generates GUI structs from ROS .msg files, so drone_state always matches the ROS messages.

    python tools/gen_embedded_msgs.py <path to vortex-msgs/msg>

Linux:   python3 tools/gen_embedded_msgs.py ~/code/ros2_ws/src/vortex-msgs/msg
Windows: python tools\\gen_embedded_msgs.py C:\\Users\\jpuer\\vortex\\msgsWORK\\vortex_msgs\\msg

Writes two files (commit both, so nobody needs Python or ROS to build):
    lib/embedded_msgs.h                  gui_ structs + enums, used by client and server
    gui-server/src/embedded_msgs_copy.h  GuiCopy() functions, ROS C struct -> gui_ struct (server only)

Do not edit the generated files by hand. Change the .msg file and run the script again.
"""

import os
import re
import sys

# ============================================================ SETTINGS ==========

# The messages the GUI should know about. Messages used inside them
# (like ServoStatus inside TorpedoStatus) are added automatically.
MESSAGES = [
    "BmsStatus",
    "DropperStatus",
    "GripperStatus",
    "HardwareKillswitch",
    "MarkerCount",
    "MotorControllerStatus",
    "PcbStatusArray",
    "PowerInterface",
    "ThrusterStatusArray",
    "TorpedoCount",
    "TorpedoStatus",
]

# Arrays without a fixed length (Type[]) contain a pointer in ROS and can not be sent as raw bytes.
# The GUI gets a fixed array of this length plus a Count field instead.
# Key: (message, field)
MAX_LENGTH = {
    ("ThrusterStatusArray", "thrusters"): 8,
}

PACKAGE = "vortex_msgs"

# ROS type -> type in typedef.h
PRIMITIVES = {
    "bool": "u8", "byte": "u8", "char": "u8",
    "int8": "s8", "uint8": "u8",
    "int16": "s16", "uint16": "u16",
    "int32": "s32", "uint32": "u32",
    "int64": "s64", "uint64": "u64",
    "float32": "f32", "float64": "f64",
}

# Types from other packages that the GUI knows about. Value: (gui struct, ROS C struct, ROS C header)
EXTERNAL = {
    "std_msgs/Header": ("gui_header", "std_msgs__msg__Header", "std_msgs/msg/header.h"),
}

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_HEADER = os.path.join(ROOT, "lib", "embedded_msgs.h")
OUT_COPY = os.path.join(ROOT, "gui-server", "src", "embedded_msgs_copy.h")

# ============================================================ HELPERS ==========


def snake(name):
    """ThrusterStatusArray -> thruster_status_array (same rule as ROS uses for header names)."""
    s = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", name)
    s = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", s)
    return s.lower()


def pascal(name):
    """fault_status -> FaultStatus (field style in drone_state.h)."""
    return "".join(p[:1].upper() + p[1:] for p in name.split("_"))


def gui_struct(msg):
    return "gui_" + snake(msg)


def fail(text):
    print("ERROR: " + text)
    sys.exit(1)


# ============================================================ PARSING ==========


def parse_msg(path, name):
    """Reads one .msg file. Returns {"constants": [...], "fields": [...]}."""
    constants, fields = [], []
    for number, line in enumerate(open(path, encoding="utf8"), start=1):
        code, _, comment = line.partition("#")
        code, comment = code.strip(), comment.strip()
        if not code:
            continue
        where = "%s.msg line %d" % (name, number)

        # Constant:  uint8 HEALTHY = 1
        m = re.match(r"^(\w+)\s+([A-Z][A-Z0-9_]*)\s*=\s*(\S+)$", code)
        if m:
            if m.group(1) not in PRIMITIVES:
                fail(where + ": only number constants are supported")
            constants.append({"name": m.group(2), "value": m.group(3), "comment": comment})
            continue

        # Field:  Type name   Type[4] name   Type[] name   Type[<=4] name
        m = re.match(r"^([\w/]+)(\[(<=)?(\d*)\])?\s+([a-z][a-z0-9_]*)(\s+\S+)?$", code)
        if not m:
            fail(where + ": could not read '%s'" % code)
        if m.group(6):
            fail(where + ": default values are not supported")
        typ, is_array, bounded, length, field = m.group(1), m.group(2), m.group(3), m.group(4), m.group(5)
        if typ.startswith(PACKAGE + "/"):
            typ = typ[len(PACKAGE) + 1:]
        if typ in ("string", "wstring"):
            fail(where + ": strings have a pointer and can not be sent as raw bytes")

        array = None  # None, ("fixed", N) or ("sequence", N)
        if is_array:
            if length and not bounded:
                array = ("fixed", int(length))
            else:
                maximum = MAX_LENGTH.get((name, field)) or (int(length) if length else None)
                if not maximum:
                    fail(where + ": '%s' has no fixed length. Add (\"%s\", \"%s\") to MAX_LENGTH" % (field, name, field))
                array = ("sequence", maximum)
        fields.append({"type": typ, "name": field, "array": array, "comment": comment})
    return {"constants": constants, "fields": fields}


def load_all(msg_dir):
    """Reads all wanted messages and the ones they use. Returns them in an order where
    a message always comes after the messages it uses (C needs that)."""
    msgs, order = {}, []

    def visit(name, chain):
        if name in order:
            return
        if name in chain:
            fail("message uses itself: " + " -> ".join(chain + [name]))
        if name not in msgs:
            path = os.path.join(msg_dir, name + ".msg")
            if not os.path.isfile(path):
                fail("can not find " + path)
            msgs[name] = parse_msg(path, name)
        for f in msgs[name]["fields"]:
            t = f["type"]
            if t in PRIMITIVES or t in EXTERNAL:
                continue
            if "/" in t:
                fail("%s.msg: type '%s' from another package. Add it to EXTERNAL" % (name, t))
            visit(t, chain + [name])
        order.append(name)

    for name in MESSAGES:
        visit(name, [])
    return msgs, order


# ============================================================ OUTPUT ==========

BANNER = """// GENERATED by tools/gen_embedded_msgs.py from the .msg files in vortex_msgs.
// DO NOT EDIT. Change the .msg file and run the script again.
"""


def write_header(msgs, order):
    out = [BANNER,
           "#ifndef embedded_msgs_h",
           "#define embedded_msgs_h",
           '#include "typedef.h"',
           "",
           "// std_msgs/Header without frame_id (a string can not be sent as raw bytes)",
           "struct gui_header {",
           "  s32 Sec;",
           "  u32 Nanosec;",
           "};",
           ""]
    for name in order:
        msg = msgs[name]
        out.append("//==================================================== %s.msg" % name)
        if msg["constants"]:
            out.append("enum {")
            for c in msg["constants"]:
                line = "  gui_%s__%s = %s," % (name, c["name"], c["value"])
                out.append(line + ("  // " + c["comment"] if c["comment"] else ""))
            out.append("};")
        out.append("struct %s {" % gui_struct(name))
        for f in msg["fields"]:
            t = f["type"]
            ctype = PRIMITIVES.get(t) or (EXTERNAL[t][0] if t in EXTERNAL else gui_struct(t))
            comment = ("  // " + f["comment"]) if f["comment"] else ""
            if f["array"] is None:
                out.append("  %s %s;%s" % (ctype, pascal(f["name"]), comment))
            elif f["array"][0] == "fixed":
                out.append("  %s %s[%d];%s" % (ctype, pascal(f["name"]), f["array"][1], comment))
            else:
                out.append("  %s %s[%d];%s" % (ctype, pascal(f["name"]), f["array"][1], comment))
                out.append("  u32 %sCount;  // Number of valid elements in %s" % (pascal(f["name"]), pascal(f["name"])))
        out.append("};")
        out.append("")
    out.append("#endif")
    return "\n".join(out) + "\n"


def write_copy(msgs, order):
    out = [BANNER,
           "// Server only: copies a ROS message (C struct from rcl) into the gui_ struct.",
           "// Use: GuiCopy(&DroneState->Bms[0], &Msg);",
           "#ifndef embedded_msgs_copy_h",
           "#define embedded_msgs_copy_h",
           '#include "../../lib/embedded_msgs.h"']
    for t in EXTERNAL.values():
        out.append("#include <%s>" % t[2])
    for name in order:
        out.append("#include <%s/msg/%s.h>" % (PACKAGE, snake(name)))
    out += ["",
            "static void GuiCopy(gui_header *Dst, const std_msgs__msg__Header *Src) {",
            "  Dst->Sec = Src->stamp.sec;",
            "  Dst->Nanosec = Src->stamp.nanosec;",
            "}",
            ""]
    for name in order:
        out.append("static void GuiCopy(%s *Dst, const %s__msg__%s *Src) {" % (gui_struct(name), PACKAGE, name))
        for f in msgs[name]["fields"]:
            dst, src = "Dst->" + pascal(f["name"]), "Src->" + f["name"]
            simple = f["type"] in PRIMITIVES
            if f["array"] is None:
                out.append("  %s = %s;" % (dst, src) if simple else "  GuiCopy(&%s, &%s);" % (dst, src))
            elif f["array"][0] == "fixed":
                body = "%s[I] = %s[I];" % (dst, src) if simple else "GuiCopy(&%s[I], &%s[I]);" % (dst, src)
                out.append("  for (u32 I = 0; I < %d; I++) %s" % (f["array"][1], body))
            else:
                n = f["array"][1]
                out.append("  %sCount = %s.size < %d ? (u32)%s.size : %d;" % (dst, src, n, src, n))
                body = ("%s[I] = %s.data[I];" % (dst, src) if simple
                        else "GuiCopy(&%s[I], &%s.data[I]);" % (dst, src))
                out.append("  for (u32 I = 0; I < %sCount; I++) %s" % (dst, body))
        out.append("}")
        out.append("")
    out.append("#endif")
    return "\n".join(out) + "\n"


def save(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf8", newline="\n") as f:
        f.write(text)
    print("wrote " + os.path.relpath(path, ROOT))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    msgs, order = load_all(os.path.expanduser(sys.argv[1]))
    save(OUT_HEADER, write_header(msgs, order))
    save(OUT_COPY, write_copy(msgs, order))
