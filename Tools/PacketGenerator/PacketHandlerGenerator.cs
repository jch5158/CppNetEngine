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
        public static bool Generate(eRole role, string delimiter, string protoName, string protoDirPath,
            string outputDirPath)
        {
            string filePath = Path.Combine(protoDirPath, $"{protoName}.desc");

            if (!File.Exists(filePath))
            {
                Console.WriteLine($"[Error] 파일을 찾을 수 없습니다! 경로를 확인하세요.");
                return false;
            }

            if (!GenerateInitHandleString(role, delimiter, $"{protoName}.proto", filePath, out var initHandleString))
            {
                return false;
            }

            if (!GenerateHandleFunctionDeclares(role, delimiter, $"{protoName}.proto", filePath,
                    out var handleFunctionDeclareString))
            {
                return false;
            }

            var handleFileContent = string.Format(PacketFormatter.HANDLE_FILE_FORMAT, protoName, role,
                initHandleString,
                handleFunctionDeclareString);

            try
            {
                File.WriteAllText(outputDirPath, handleFileContent);
            }
            catch (Exception e)
            {
                Console.WriteLine($"[Generate] 패킷 핸들러 생성 중 오류 발생: {e.Message}");
                return false;
            }

            return true;
        }

        private static bool GenerateInitHandleString(eRole role, string delimiter, string protoName, string filePath,
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
                        if (!packetName.Contains(delimiter))
                        {
                            continue;
                        }

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

        private static bool GenerateHandleFunctionDeclares(eRole role, string delimiter, string protoName, string filePath,
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
                        if (!packetName.Contains(delimiter))
                        {
                            continue;
                        }

                        handleFunctionDeclareString += string.Format(PacketFormatter.DECLARE_FILE_FORMAT, packetName);
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
    }
}
