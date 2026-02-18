using PacketGenerator;
using Protocol;

// 2. 상대 경로 조합 (요청하신 경로 적용)
// Common/Protocol 폴더 안의 Protocol.desc 파일을 가리킴

// 1. 실행 파일이 있는 절대 폴더 경로 가져오기 (예: C:\Tools\Generator\)
var baseDirPath = AppDomain.CurrentDomain.BaseDirectory;
var protoPath = Path.Combine(baseDirPath, @"..\..\..\..\Common\Protocol");
var outputPath = Path.Combine(baseDirPath, @"..\..\..\..\DummyClient\ClientLoginPacketHandler.h");

if(PacketHandlerGenerator.Generate(eRole.Client, "S2C", "Login", protoPath, outputPath) == true)
{
    Console.WriteLine("SUCCESS");
}
