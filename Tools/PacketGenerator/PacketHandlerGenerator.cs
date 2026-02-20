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
        public static bool Generate(PacketProjectsConfig config, string baseDirPath)
        {
            foreach (var projectReceiver in config.Projects)
            {
                foreach (var projectSender in config.Projects)
                {
                    if (projectReceiver.Name == projectSender.Name)
                    {
                        continue;
                    }

                    if (!Enum.TryParse<eRole>(projectReceiver.Role, true, out var receiver) || !Enum.TryParse<eRole>(projectSender.Role, true, out var sender))
                    {
                        Console.WriteLine($"[Error] 프로젝트 '{projectReceiver.Name}'의 역할 '{projectReceiver.Role}'이(가) 유효하지 않습니다.");
                        continue;
                    }

                    var protoPath = Path.Combine(baseDirPath, @"..\..\..\..\Common\Protocol");
                    var outputPath = Path.Combine(baseDirPath, @$"..\..\..\..\{projectReceiver.Name}\Generated");
                    if (!GenerateFile(receiver, sender, projectReceiver.Name, projectSender.Name, protoPath, outputPath))
                    {
                        Console.WriteLine("GenerateFile is Failed");
                    }
                }
            }

            return true;
        }

        public static bool GenerateFile(eRole receiver, eRole sender, string prjReceiverName, string prjSenderName,
            string protoDirPath, string outputDirPath)
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

                var fileName = $"{sender.ToString()}{protoName.ToString()}PacketHandler.h";
                var outputFilePath = Path.Combine(outputDirPath, fileName);

                if (!GenerateInitHandleString(receiver, sender, $"{protoName}.proto", protoFilePath, out var initHandleString))
                {
                    return false;
                }

                if (!GenerateHandleFunctionDeclares(receiver, sender, $"{protoName}.proto", protoFilePath,
                        out var handleFunctionDeclareString))
                {
                    return false;
                }

                if (!GenerateMakeSendBufferFunction(receiver, sender, $"{protoName}.proto", protoFilePath,
                        out var makeSendBufferFunctionString))
                {
                    return false;
                }

                var handleFileContent = string.Format(PacketFormatter.HANDLE_FILE_FORMAT, sender.ToString(), protoName,
                    initHandleString,
                    handleFunctionDeclareString,
                    makeSendBufferFunctionString);

                try
                {
                    var directoryPath = Path.GetDirectoryName(outputFilePath);

                    if (!string.IsNullOrEmpty(directoryPath) && !Directory.Exists(directoryPath))
                    {
                        Directory.CreateDirectory(directoryPath);
                    }

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

        private static bool GenerateInitHandleString(eRole receiver, eRole sender, string protoName, string filePath,
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

                        if (options.GetExtension(PacketIdExtensions.Receiver) != receiver && options.GetExtension(PacketIdExtensions.Sender) != sender)
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

        private static bool GenerateHandleFunctionDeclares(eRole receiver, eRole sender, string protoName, string filePath,
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
                        if (options.GetExtension(PacketIdExtensions.Receiver) != receiver && options.GetExtension(PacketIdExtensions.Sender) != sender)
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

        private static bool GenerateMakeSendBufferFunction(eRole receiver, eRole sender, string protoName, string filePath,
            out string makeSendBufferFunctionString)
        {
            makeSendBufferFunctionString = "";
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

                        if (options.GetExtension(PacketIdExtensions.Receiver) != receiver && options.GetExtension(PacketIdExtensions.Sender) != sender)
                        {
                            continue;
                        }

                        var packetName = msg.Name;
                        makeSendBufferFunctionString += string.Format(PacketFormatter.MAKE_SEND_BUFFER_FUNCTION_FORMAT,
                            packetName, $"ID_{packetName}");
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine($"[GenerateMakeSendBufferFunction] 패킷 핸들러 생성 중 오류 발생: {e.Message}");
                return false;
            }

            return true;

        }

    }
}
