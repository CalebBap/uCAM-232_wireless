#ifndef CameraCommands_h
#define CameraCommands_h

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <string>
#include <string_view>
#include <map>
#include <cstdint>

class CameraCommands {
    public:
        explicit CameraCommands(WebSocketsServer& wbs) : webSocket(wbs) {}

        void attemptSync();
        void unrecognisedCommand(std::string command);
        void attemptInitialisation(std::string command);
        void attemptSnapshot(std::string command);

    private:
        enum DataTypeValues { SNAPSHOT = 0x01, RAW_PREVIEW = 0x02, JPEG_PREVIEW = 0x05 };

        enum ColourTypeIndex { RAW_2GS, RAW_4GS, RAW_8GS, RAW_8C, RAW_12C, RAW_16C, JPEG };
        static const constexpr char* const ColourTypes[] = { "2GS", "4GS", "8GS", "8C", "12C", "16C", "J" };

        enum ResolutionTypeIndex { RAW_80x60, RAW_160x120, RAW_320x240, RAW_640x480, RAW_128x128, RAW_128x96, 
                                    JPEG_80x64, JPEG_160x128, JPEG_320x240, JPEG_640x480 };
        static const constexpr char* const ResolutionTypes[] = { "80x60", "160x120", "320x240", "640x480", "128x128", 
                                                                    "128x96", "80x64", "160x128", "320x240", "640x480" };

        std::map<std::string_view, byte> ColourTypeValues = {  
            {ColourTypes[RAW_2GS], 0x01}, {ColourTypes[RAW_4GS], 0x02}, {ColourTypes[RAW_8GS], 0x03},
            {ColourTypes[RAW_8C], 0x04}, {ColourTypes[RAW_12C], 0x05}, {ColourTypes[RAW_16C], 0x06},
            {ColourTypes[JPEG], 0x07}
        };

        std::map<std::string_view, byte> RawResolutionValues = { 
            {ResolutionTypes[RAW_80x60], 0x01}, {ResolutionTypes[RAW_160x120], 0x03},
            {ResolutionTypes[RAW_320x240], 0x05}, {ResolutionTypes[RAW_640x480], 0x07},
            {ResolutionTypes[RAW_128x128], 0x09}, {ResolutionTypes[RAW_128x96], 0x0B}
        };

        std::map<std::string_view, byte> JpegResolutionValues = {  
            {ResolutionTypes[JPEG_80x64], 0x01}, {ResolutionTypes[JPEG_160x128], 0x03},
            {ResolutionTypes[JPEG_320x240], 0x05}, {ResolutionTypes[JPEG_640x480], 0x07}
        };

        const std::map<uint8_t, std::string> NakReason {
            { 0x01, "Picture Type Error" }, { 0x02, "Picture Up Scale" },
            { 0x03, "Picture Scale Error" }, { 0x04, "Unexpected Reply" },
            { 0x05, "Send Picture Timeout" }, { 0x06, "Unexpected Command" },
            { 0x07, "SRAM JPEG Type Error" }, { 0x08, "SRAM JPEG Size Error" },
            { 0x09, "Picture Format Error" }, { 0x0A, "Picture Size Error" },
            { 0x0B, "Parameter Error" }, { 0x0C, "Send Register Timeout" },
            { 0x0D, "Command ID Error" }, { 0x0F, "Picture Not Ready" },
            { 0x10, "Transfer Package Number Error" }, { 0x11, "Set Transfer Package Size Wrong" },
            { 0xF0, "Command Header Error" }, { 0xF1, "Command Length Error" },
            { 0xF5, "Send Picture Error" }, { 0xFF, "Send Command Error" }
        };

        WebSocketsServer& webSocket;

        const char* current_colour_type { nullptr };
        const char* current_resolution { nullptr };

        static constexpr int CMD_CLIENT_MESSAGE_SIZE { 37 };
        static constexpr int PKG_SIZE_BYTES { 512 };
        static constexpr int NUM_PKG_DATA_BYTES { PKG_SIZE_BYTES - 6 };
        static constexpr int MIN_PKG_BYTES { 5 };
        static constexpr int NUM_BYTES_IN_CMD { 6 };
        static constexpr int MAX_CMD_ATTEMPTS { 60 };
        static constexpr int MIN_CMD_DELAY { 5 };

        static constexpr std::string_view cmd_delimiter = ":";
        static constexpr std::string_view value_delimiter = ",";
        static constexpr const char* hex_digits = "0123456789abcdef";

        const std::string sync_ack = "#sync_ack";
        const std::string sync_nak = "#sync_nak";
        const std::string init_ack = "#init_ack";
        const std::string init_nak = "#init_nak";
        const std::string snap_ack = "#snap_ack";
        const std::string snap_nak = "#snap_nak";

        static constexpr byte init_id { 0x01 };
        static constexpr byte snapshot_id { 0x05 };
        static constexpr byte pkg_size_id { 0x06 };
        static constexpr byte sync_id { 0x0D };
        static constexpr byte picture_id { 0x04 };
        static constexpr byte data_id { 0x0A };
        static constexpr byte nak_byte { 0x04 };
        static constexpr byte JpegSnapshot { 0x00 };
        static constexpr byte RawSnapshot { 0x01 };

        static constexpr byte nak_reply[] = { 0xAA, 0x0F, 0x00 };

        void sendClientMessage(std::string message);
        void sendClientCommand(const byte* cmd);
        void receiveCameraResponse(byte* reply, int reply_size);
        bool sendCameraCommand(const byte* cmd, const byte id);
        bool getCameraCommand(const byte id, uint8_t& nak_reason);

        int getColourTypeIndex(const std::string& colour_type_str);
        int getResolutionTypeIndex(const int colour_type_index, const std::string& resolution);

        int parseSnapshotParameters(std::string command);
        bool parseInitParameters(byte* init_cmd, std::string command);

        bool setPackageSize();
        void getPicture(DataTypeValues data_type);
        void getData(DataTypeValues data_type);
        void getJpegData(uint32_t img_size);
        void getRawData(uint32_t img_size);
};

#endif