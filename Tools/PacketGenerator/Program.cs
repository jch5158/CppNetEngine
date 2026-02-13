using Google.Protobuf.Reflection;

var exePath = AppDomain.CurrentDomain.BaseDirectory;

// 2. 상대 경로 조합 (요청하신 경로 적용)
// Common/Protocol 폴더 안의 Protocol.desc 파일을 가리킴
const string relativePath = "../../../../Common/Protocol/Login.desc";
var fullPath = Path.GetFullPath(Path.Combine(exePath, relativePath));

Console.WriteLine($"[Info] 파일 경로 확인: {fullPath}");

// 3. 파일 존재 여부 체크
if (!File.Exists(fullPath))
{
    Console.WriteLine($"[Error] 파일을 찾을 수 없습니다! 경로를 확인하세요.");
    return;
}

// 4. .desc 파일 로딩 (핵심)
try
{
    using var stream = File.OpenRead(fullPath);
    // 바이너리 데이터를 FileDescriptorSet 객체로 파싱
    var descriptorSet = FileDescriptorSet.Parser.ParseFrom(stream);

    Console.WriteLine($"[Success] 로딩 성공! 총 {descriptorSet.File.Count}개의 파일 정보가 포함됨.");

    // (확인용) 첫 번째 파일 이름만 출력해보기
    if (descriptorSet.File.Count > 0)
    {
        Console.WriteLine($" - 첫 번째 파일명: {descriptorSet.File[0].Name}");
    }

    foreach (var file in descriptorSet.File)
    {
        // [중요] google/protobuf 같은 내장 파일은 건너뛰기
        if (file.Name.Contains("google/protobuf"))
            continue;

        Console.WriteLine($"====== 파일명: {file.Name} ======");

        // 파일 안에 있는 모든 메시지(struct/class) 순회
        foreach (var message in file.MessageType)
        {
            // ★ 여기서 이름을 가져옵니다
            var packetName = message.Name;

            Console.WriteLine($"패킷 이름: {packetName}");

            // 예: "C_"로 시작하는 것만 찾고 싶다면?
            if (packetName.StartsWith("C2S") || packetName.StartsWith("S2C"))
            {
                Console.WriteLine($" -> 이건 진짜 패킷이네요: {packetName}");
            }
        }
    }

}
catch (Exception e)
{
    Console.WriteLine($"[Error] 파일 파싱 실패: {e.Message}");
}

