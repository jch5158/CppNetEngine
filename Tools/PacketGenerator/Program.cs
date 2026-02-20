using PacketGenerator;

var baseDirPath = AppDomain.CurrentDomain.BaseDirectory;
var configDirPath = Path.Combine(baseDirPath, @"..\..\Config\PacketConfig.json");
var resultConfig = PacketConfig.Load(configDirPath);

if (PacketHandlerGenerator.Generate(resultConfig, baseDirPath))
{
    Console.WriteLine("SUCCESS");
}