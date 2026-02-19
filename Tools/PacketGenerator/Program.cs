using PacketGenerator;
using Protocol;

// 2. 상대 경로 조합 (요청하신 경로 적용)
// Common/Protocol 폴더 안의 Protocol.desc 파일을 가리킴

// 1. 실행 파일이 있는 절대 폴더 경로 가져오기 (예: C:\Tools\Generator\)
var baseDirPath = AppDomain.CurrentDomain.BaseDirectory;
var configDirPath = Path.Combine(baseDirPath, @"..\..\Config\PacketConfig.json");
var resultConfig = PacketConfig.Load(configDirPath);

foreach (var project in resultConfig.Projects)
{
    if(!Enum.TryParse<eRole>(project.Role, true, out var role))
    {
        Console.WriteLine($"[Error] 프로젝트 '{project.Name}'의 역할 '{project.Role}'이(가) 유효하지 않습니다.");
        continue;
    }

    var protoPath = Path.Combine(baseDirPath, @"..\..\..\..\Common\Protocol");
    var outputPath = Path.Combine(baseDirPath, @$"..\..\..\..\{project.Name}\Generated");
    if (PacketHandlerGenerator.GenerateFile(role, project.Name, protoPath, outputPath))
    {
        Console.WriteLine("SUCCESS");
    }
}

