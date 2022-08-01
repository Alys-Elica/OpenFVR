class Uncompress:
    def __init__(self, compressionLevel):
        self.compressionLevel = compressionLevel
    
    def decode(self, data : bytes) -> bytearray:
        if self.compressionLevel == 3:
            output = bytearray()

            index = 0

            while index < len(data) - 1:
                byte = data[index]
                index = index + 1

                if (byte & 0b10000000):
                    size = (byte & 0x3F) + 1
                    tmpIndex = 0
                    if (byte & 0b01000000):
                        tmpIndex = data[index]
                        index = index + 1
                    else:
                        tmpIndex = (data[index] << 8) | data[index + 1]
                        index = index + 2

                    tmpIndex = len(output) - (tmpIndex + 1)

                    for i in range(size):
                        output.append(output[i + tmpIndex])
                else:
                    for i in range(byte + 1):
                        output.append(data[index])
                        index = index + 1

            return output
        else:
            print("Unknown compression level")
