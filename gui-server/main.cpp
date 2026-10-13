#include "../lib/typedef.h"
#include "../lib/platform.h"
#include "../lib/drone_state.h"

#include<math.h>


s32 main(s32 CharC, char **ArgV){
    PlatformInitializeNetworkLib();

    char* TestMessage = "TestMessage";
    u8 RecvBuffer[1024] = {};

    platform_ip LocalHostIP = PlatformIP("127.0.0.1");


    drone_state DS = {};
    s32 MS = 0;
    s32 SleepTime = 5;
    platform_socket ListenSocket = PlatformBindAddressAndListen(LocalHostIP, 2100);
    while (ListenSocket.IsValid) {
        platform_socket AcceptedSocket = PlatformAccept(&ListenSocket);


    while (AcceptedSocket.IsValid) {
        DS.Speed.X = 1.0f + sinf((f32)MS/10000);
        DS.Speed.Y = 0.1f + 0.3f*cosf((f32)MS/35000);
        DS.Speed.Y = 0.0f + 0.1f*cosf((f32)MS / 70000);

        DS.Pos.X += DS.Speed.X;
        DS.Pos.Y += DS.Speed.Y;
        DS.Pos.Z += DS.Speed.Z;

        DS.Thrusters[0].Speed = DS.Speed.Y * 10;
        DS.Depth = 2.0f + sinf((f32)MS / 10000);

        s64 SendtBytes = PlatformSend(&AcceptedSocket, (u8*)&DS, sizeof(DS));

        MS += SleepTime;
        PlatformSleepMS(SleepTime);
    }
    }
    




    PlatformFreeNetworkLib();
    return 0;
}