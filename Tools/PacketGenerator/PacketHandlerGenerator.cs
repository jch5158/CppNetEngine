using Google.Protobuf;
using Google.Protobuf.Reflection;
using Protocol;
using System;
using System.Collections.Generic;
using System.Data;
using System.Text;

namespace PacketGenerator
{
    internal class PacketHandlerGenerator
    {
        public static bool Generate()
        {
            return true;
        }

        public static bool GenerateFile(eRole role, string protoDirPath,
            string outputDirPath)
        {
            var descFiles = Directory.GetFiles(protoDirPath, "*.desc");
            if (descFiles.Length == 0)
            {
                Console.WriteLine($"[Error] '{protoDirPath}' 경로에서 .desc 파일을 찾을 수 없습니다.");
                return false;
            }

            foreach (var protoFilePath in descFiles)
            {
                var protoName = Path.GetFileNameWithoutExtension(protoFilePath);
                if (protoName is "Enum" or "PacketId" or "Struct")
                {
                    continue;
                }

                var fileName = $"{role.ToString()}{protoName}PacketHandler.h";
                var outputFilePath = Path.Combine(outputDirPath, fileName);

                if (!GenerateInitHandleString(role, $"{protoName}.proto", protoFilePath, out var initHandleString))
                {
                    return false;
                }

                if (!GenerateHandleFunctionDeclares(role, $"{protoName}.proto", protoFilePath,
                        out var handleFunctionDeclareString))
                {
                    return false;
                }

                var handleFileContent = string.Format(PacketFormatter.HANDLE_FILE_FORMAT, protoName, role.ToString(),
                    initHandleString,
                    handleFunctionDeclareString);

                try
                {
                    File.WriteAllText(outputFilePath, handleFileContent);
                }
                catch (Exception e)
                {
                    Console.WriteLine($"[Generate] 패킷 핸들러 생성 중 오류 발생: {e.Message}");
                    return false;
                }
            }

            return true;
        }

        private static bool GenerateInitHandleString(eRole role, string protoName, string filePath,
            out string initHandleString)
        {
            initHandleString = "";

            try
            {
                // 1. 레지스트리 생성 및 등록 (이 코드가 없으면 무조건 Unknown으로 빠집니다!)
                var registry = new ExtensionRegistry
                {
                    PacketIdExtensions.Sender,
                    PacketIdExtensions.Receiver
                };

                using var stream = File.OpenRead(filePath);
                var descriptorSet = FileDescriptorSet.Parser.WithExtensionRegistry(registry).ParseFrom(stream);
                foreach (var fileProto in descriptorSet.File)
                {
                    if (!fileProto.Name.EndsWith(protoName))
                    {
                        continue;
                    }

                    foreach (var msg in fileProto.MessageType)
                    {
                        var options = msg.Options;
                        if (options == null)
                        {
                            continue;
                        }

                        if (options.GetExtension(PacketIdExtensions.Receiver) != role)
                        {
                            continue;
                        }

                        var packetName = msg.Name;
                        initHandleString += string.Format(PacketFormatter.INIT_FILE_FORMAT, "ID_" + packetName,
                            packetName);
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine($"[GenerateInitHandleString] 패킷 핸들러 생성 중 오류 발생: {e.Message}");

                return false;
            }

            return true;
        }

        private static bool GenerateHandleFunctionDeclares(eRole role, string protoName, string filePath,
            out string handleFunctionDeclareString)
        {
            handleFunctionDeclareString = "";

            try
            {
                // 1. 레지스트리 생성 및 등록 (이 코드가 없으면 무조건 Unknown으로 빠집니다!)
                var registry = new ExtensionRegistry
                {
                    PacketIdExtensions.Sender,
                    PacketIdExtensions.Receiver
                };

                using var stream = File.OpenRead(filePath);
                var descriptorSet = FileDescriptorSet.Parser.WithExtensionRegistry(registry).ParseFrom(stream);

                foreach (var fileProto in descriptorSet.File)
                {
                    if (!fileProto.Name.EndsWith(protoName))
                    {
                        continue;
                    }

                    foreach (var msg in fileProto.MessageType)
                    {
                        var options = msg.Options;
                        if (options.GetExtension(PacketIdExtensions.Receiver) != role)
                        {
                            continue;
                        }

                        var packetName = msg.Name;
                        handleFunctionDeclareString += string.Format(PacketFormatter.DECLARE_FILE_FORMAT, packetName);
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine($"[GenerateHandleFunctionDeclares] 패킷 핸들러 생성 중 오류 발생: {e.Message}");

                return false;
            }

            return true;
        }
    }
}
