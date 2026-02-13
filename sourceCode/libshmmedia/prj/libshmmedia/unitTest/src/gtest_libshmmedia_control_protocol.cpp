/*********************************************************
 *  Copyright 2025 TVU Networks
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *********************************************************/

#include <gtest/gtest.h>
#include "libshmmedia_control_protocol.h"
#include <cstring>
#include <vector>

class LibShmMediaControlProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        handle = LibTvuMediaControlHandleCreate();
    }

    void TearDown() override {
        if (handle) {
            LibTvuMediaControlHandleDestory(handle);
            handle = nullptr;
        }
    }

    libtvumedia_control_handle_t handle;

    void initCmdData(libtvumedia_ctrlcmd_data_t& cmd) {
        memset(&cmd, 0, sizeof(cmd));
        cmd.u_structSize = sizeof(libtvumedia_ctrlcmd_data_v1_t);
    }
};

// ===================== Handle Creation/Destruction Tests =====================

TEST_F(LibShmMediaControlProtocolTest, CreateHandle) {
    libtvumedia_control_handle_t h = LibTvuMediaControlHandleCreate();
    ASSERT_NE(h, nullptr) << "Handle creation should succeed";
    LibTvuMediaControlHandleDestory(h);
}

TEST_F(LibShmMediaControlProtocolTest, DestroyNullHandle) {
    LibTvuMediaControlHandleDestory(nullptr);
    // Should not crash
}

// ===================== Insert Key Frame Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadInsertKeyFrame) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdInsertKF;
    cmd.o_params.o_insertKF.u_program_index = 5;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0) << "Write should succeed";
    ASSERT_NE(pOut, nullptr) << "Output pointer should not be null";

    // Read back
    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    ASSERT_NE(readHandle, nullptr);

    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0) << "Read should succeed";
    ASSERT_EQ(counts, 1) << "Should have 1 command";
    ASSERT_NE(pParams, nullptr);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdInsertKF);
    EXPECT_EQ(pParams[0].o_params.o_insertKF.u_program_index, 5);

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Change Bitrate Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadChangeBitrate) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdChangeBitRate;
    cmd.o_params.o_changeBitrate.u_vbitrate = 5000000;
    cmd.o_params.o_changeBitrate.u_abitrate = 128000;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeBitRate);
    EXPECT_EQ(pParams[0].o_params.o_changeBitrate.u_vbitrate, 5000000u);
    EXPECT_EQ(pParams[0].o_params.o_changeBitrate.u_abitrate, 128000u);

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Change Resolution Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadChangeResolution) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdChangeResolution;
    cmd.o_params.o_changeResolution.u_width = 1920;
    cmd.o_params.o_changeResolution.u_height = 1080;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeResolution);
    EXPECT_EQ(pParams[0].o_params.o_changeResolution.u_width, 1920);
    EXPECT_EQ(pParams[0].o_params.o_changeResolution.u_height, 1080);

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Change Program Bitrate Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadChangeProgramBitrate) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdChangeProgramBitRate;
    cmd.o_params.o_progBitrate.u_programIndex = 2;
    cmd.o_params.o_progBitrate.u_vbitrate = 8000000;
    cmd.o_params.o_progBitrate.u_abitrate = 256000;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeProgramBitRate);
    EXPECT_EQ(pParams[0].o_params.o_progBitrate.u_programIndex, 2);
    EXPECT_EQ(pParams[0].o_params.o_progBitrate.u_vbitrate, 8000000u);
    EXPECT_EQ(pParams[0].o_params.o_progBitrate.u_abitrate, 256000u);

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Change Audio Params Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadChangeAudioParams) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdChangeAudioParams;
    cmd.o_params.o_audio.u_audio_only = 1;
    cmd.o_params.o_audio.u_aac_profile = 2;
    cmd.o_params.o_audio.u_tracks_layout = 0x0062;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeAudioParams);
    EXPECT_EQ(pParams[0].o_params.o_audio.u_audio_only, 1);
    EXPECT_EQ(pParams[0].o_params.o_audio.u_aac_profile, 2);
    EXPECT_EQ(pParams[0].o_params.o_audio.u_tracks_layout, 0x0062ull);

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Change Delay Params Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadChangeDelayParams) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdChangeDelayParams;
    cmd.o_params.o_delay.u_delay_ms = 5000;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeDelayParams);
    EXPECT_EQ(pParams[0].o_params.o_delay.u_delay_ms, 5000u);

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Send Audio Volumes Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadSendAudioVolumes) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdSendAudioVolumeParams;
    cmd.o_params.o_audioVolume.u_counts = 4;
    cmd.o_params.o_audioVolume.u_volume[0] = 100;
    cmd.o_params.o_audioVolume.u_volume[1] = 80;
    cmd.o_params.o_audioVolume.u_volume[2] = 60;
    cmd.o_params.o_audioVolume.u_volume[3] = 40;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdSendAudioVolumeParams);
    EXPECT_EQ(pParams[0].o_params.o_audioVolume.u_counts, 4);
    EXPECT_EQ(pParams[0].o_params.o_audioVolume.u_volume[0], 100);
    EXPECT_EQ(pParams[0].o_params.o_audioVolume.u_volume[1], 80);
    EXPECT_EQ(pParams[0].o_params.o_audioVolume.u_volume[2], 60);
    EXPECT_EQ(pParams[0].o_params.o_audioVolume.u_volume[3], 40);

    LibTvuMediaControlHandleDestory(readHandle);
}

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadSendAudioVolumesMaxChannels) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdSendAudioVolumeParams;
    cmd.o_params.o_audioVolume.u_counts = 32; // Max channels

    for (int i = 0; i < 32; i++) {
        cmd.o_params.o_audioVolume.u_volume[i] = i * 10;
    }

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].o_params.o_audioVolume.u_counts, 32);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(pParams[0].o_params.o_audioVolume.u_volume[i], i * 10);
    }

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Start Live Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadStartLive) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdStartProgLiveParams;
    cmd.o_params.o_startLive.u_program_index = 1;
    cmd.o_params.o_startLive.b_vbr = 1;
    cmd.o_params.o_startLive.u_video_bit_rate = 6000000;
    cmd.o_params.o_startLive.u_audio_bit_rate = 192000;
    cmd.o_params.o_startLive.u_video_codec_fourcc = 0x34363248; // H264
    cmd.o_params.o_startLive.u_audio_codec_fourcc = 0x4341414D; // MAAC
    cmd.o_params.o_startLive.u_audio_tracks_layout = 0x0062;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdStartProgLiveParams);
    EXPECT_EQ(pParams[0].o_params.o_startLive.u_program_index, 1u);
    EXPECT_EQ(pParams[0].o_params.o_startLive.b_vbr, 1);
    EXPECT_EQ(pParams[0].o_params.o_startLive.u_video_bit_rate, 6000000u);
    EXPECT_EQ(pParams[0].o_params.o_startLive.u_audio_bit_rate, 192000u);
    EXPECT_EQ(pParams[0].o_params.o_startLive.u_video_codec_fourcc, 0x34363248u);
    EXPECT_EQ(pParams[0].o_params.o_startLive.u_audio_codec_fourcc, 0x4341414Du);
    EXPECT_EQ(pParams[0].o_params.o_startLive.u_audio_tracks_layout, 0x0062ull);

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Stop Live Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadStopLive) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdStopProgLiveParams;
    cmd.o_params.o_stopLive.u_program_index = 3;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdStopProgLiveParams);
    EXPECT_EQ(pParams[0].o_params.o_stopLive.u_program_index, 3u);

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Cameras Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadCameras) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdCamerasParams;
    cmd.o_params.o_cameras.u_index = 2;
    cmd.o_params.o_cameras.u_video_connection = 1;
    cmd.o_params.o_cameras.u_audio_connection = 2;
    cmd.o_params.o_cameras.u_video_fmt = 0x12345678;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdCamerasParams);
    EXPECT_EQ(pParams[0].o_params.o_cameras.u_index, 2);
    EXPECT_EQ(pParams[0].o_params.o_cameras.u_video_connection, 1);
    EXPECT_EQ(pParams[0].o_params.o_cameras.u_audio_connection, 2);
    EXPECT_EQ(pParams[0].o_params.o_cameras.u_video_fmt, 0x12345678u);

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Common JSON Command Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadCommonJson) {
    ASSERT_NE(handle, nullptr);

    const char* jsonStr = "{\"ver\":\"1.0\",\"pgmi\":1,\"cmds\":[{\"cmdt\":1}]}";

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdCommonJsonParams;
    cmd.o_params.o_json.p_json = jsonStr;
    cmd.o_params.o_json.u_len = strlen(jsonStr);

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdCommonJsonParams);
    EXPECT_EQ(pParams[0].o_params.o_json.u_len, strlen(jsonStr));
    ASSERT_NE(pParams[0].o_params.o_json.p_json, nullptr);
    EXPECT_EQ(memcmp(pParams[0].o_params.o_json.p_json, jsonStr, strlen(jsonStr)), 0);

    LibTvuMediaControlHandleDestory(readHandle);
}

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadLargeJson) {
    ASSERT_NE(handle, nullptr);

    // Create a larger JSON string
    std::string jsonStr = "{\"ver\":\"1.0\",\"data\":\"";
    for (int i = 0; i < 1000; i++) {
        jsonStr += "abcdefghij";
    }
    jsonStr += "\"}";

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdCommonJsonParams;
    cmd.o_params.o_json.p_json = jsonStr.c_str();
    cmd.o_params.o_json.u_len = jsonStr.length();

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 1);

    EXPECT_EQ(pParams[0].o_params.o_json.u_len, jsonStr.length());

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Multiple Commands Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadMultipleCommands) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmds[3];

    // Command 1: Insert Key Frame
    initCmdData(cmds[0]);
    cmds[0].u_command_type = kLibTvuMediaCtrlCmdInsertKF;
    cmds[0].o_params.o_insertKF.u_program_index = 1;

    // Command 2: Change Bitrate
    initCmdData(cmds[1]);
    cmds[1].u_command_type = kLibTvuMediaCtrlCmdChangeBitRate;
    cmds[1].o_params.o_changeBitrate.u_vbitrate = 4000000;
    cmds[1].o_params.o_changeBitrate.u_abitrate = 96000;

    // Command 3: Change Resolution
    initCmdData(cmds[2]);
    cmds[2].u_command_type = kLibTvuMediaCtrlCmdChangeResolution;
    cmds[2].o_params.o_changeResolution.u_width = 1280;
    cmds[2].o_params.o_changeResolution.u_height = 720;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, cmds, 3, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 3) << "Should have 3 commands";

    // Verify command 1
    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdInsertKF);
    EXPECT_EQ(pParams[0].o_params.o_insertKF.u_program_index, 1);

    // Verify command 2
    EXPECT_EQ(pParams[1].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeBitRate);
    EXPECT_EQ(pParams[1].o_params.o_changeBitrate.u_vbitrate, 4000000u);
    EXPECT_EQ(pParams[1].o_params.o_changeBitrate.u_abitrate, 96000u);

    // Verify command 3
    EXPECT_EQ(pParams[2].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeResolution);
    EXPECT_EQ(pParams[2].o_params.o_changeResolution.u_width, 1280);
    EXPECT_EQ(pParams[2].o_params.o_changeResolution.u_height, 720);

    LibTvuMediaControlHandleDestory(readHandle);
}

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadManyCommands) {
    ASSERT_NE(handle, nullptr);

    const int numCommands = 100;
    std::vector<libtvumedia_ctrlcmd_data_t> cmds(numCommands);

    for (int i = 0; i < numCommands; i++) {
        initCmdData(cmds[i]);
        cmds[i].u_command_type = kLibTvuMediaCtrlCmdInsertKF;
        cmds[i].o_params.o_insertKF.u_program_index = i % 256;
    }

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, cmds.data(), numCommands, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, numCommands);

    for (int i = 0; i < numCommands; i++) {
        EXPECT_EQ(pParams[i].o_params.o_insertKF.u_program_index, i % 256)
            << "Command " << i << " program index mismatch";
    }

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Error Cases Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteWithInvalidStructSize) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.u_structSize = 0; // Invalid size
    cmd.u_command_type = kLibTvuMediaCtrlCmdInsertKF;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    EXPECT_LT(written, 0) << "Write with invalid struct size should fail";
}

TEST_F(LibShmMediaControlProtocolTest, ReadWithNullBuffer) {
    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    ASSERT_NE(readHandle, nullptr);

    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, nullptr, 100,
                                                   &pParams, &counts);
    EXPECT_LT(readResult, 0) << "Read with null buffer should fail";

    LibTvuMediaControlHandleDestory(readHandle);
}

TEST_F(LibShmMediaControlProtocolTest, ReadWithZeroLength) {
    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    ASSERT_NE(readHandle, nullptr);

    uint8_t buffer[10] = {0};
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, buffer, 0,
                                                   &pParams, &counts);
    EXPECT_LT(readResult, 0) << "Read with zero length should fail";

    LibTvuMediaControlHandleDestory(readHandle);
}

TEST_F(LibShmMediaControlProtocolTest, WriteWithNullHandle) {
    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdInsertKF;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(nullptr, &cmd, 1, &pOut);
    EXPECT_LT(written, 0) << "Write with null handle should fail";
}

TEST_F(LibShmMediaControlProtocolTest, ReadWithNullHandle) {
    uint8_t buffer[100] = {0};
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(nullptr, buffer, 100,
                                                   &pParams, &counts);
    EXPECT_LT(readResult, 0) << "Read with null handle should fail";
}

// ===================== Edge Values Tests =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadMaxValues) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdChangeBitRate;
    cmd.o_params.o_changeBitrate.u_vbitrate = UINT32_MAX;
    cmd.o_params.o_changeBitrate.u_abitrate = UINT32_MAX;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);

    EXPECT_EQ(pParams[0].o_params.o_changeBitrate.u_vbitrate, UINT32_MAX);
    EXPECT_EQ(pParams[0].o_params.o_changeBitrate.u_abitrate, UINT32_MAX);

    LibTvuMediaControlHandleDestory(readHandle);
}

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadZeroValues) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdChangeBitRate;
    cmd.o_params.o_changeBitrate.u_vbitrate = 0;
    cmd.o_params.o_changeBitrate.u_abitrate = 0;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);

    EXPECT_EQ(pParams[0].o_params.o_changeBitrate.u_vbitrate, 0u);
    EXPECT_EQ(pParams[0].o_params.o_changeBitrate.u_abitrate, 0u);

    LibTvuMediaControlHandleDestory(readHandle);
}

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadMax64BitValue) {
    ASSERT_NE(handle, nullptr);

    libtvumedia_ctrlcmd_data_t cmd;
    initCmdData(cmd);
    cmd.u_command_type = kLibTvuMediaCtrlCmdChangeAudioParams;
    cmd.o_params.o_audio.u_tracks_layout = UINT64_MAX;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);

    EXPECT_EQ(pParams[0].o_params.o_audio.u_tracks_layout, UINT64_MAX);

    LibTvuMediaControlHandleDestory(readHandle);
}

// ===================== Reuse Handle Tests =====================

TEST_F(LibShmMediaControlProtocolTest, ReuseHandleForMultipleWrites) {
    ASSERT_NE(handle, nullptr);

    for (int i = 0; i < 10; i++) {
        libtvumedia_ctrlcmd_data_t cmd;
        initCmdData(cmd);
        cmd.u_command_type = kLibTvuMediaCtrlCmdInsertKF;
        cmd.o_params.o_insertKF.u_program_index = i;

        const uint8_t* pOut = nullptr;
        int written = LibTvuMediaControlHandleWrite(handle, &cmd, 1, &pOut);
        ASSERT_GT(written, 0) << "Write " << i << " failed";

        libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
        const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
        int counts = 0;
        int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                       &pParams, &counts);
        ASSERT_GT(readResult, 0) << "Read " << i << " failed";
        EXPECT_EQ(pParams[0].o_params.o_insertKF.u_program_index, i);

        LibTvuMediaControlHandleDestory(readHandle);
    }
}

// ===================== All Command Types in One Test =====================

TEST_F(LibShmMediaControlProtocolTest, WriteAndReadAllCommandTypes) {
    ASSERT_NE(handle, nullptr);

    std::vector<libtvumedia_ctrlcmd_data_t> cmds(10);

    // kLibTvuMediaCtrlCmdInsertKF
    initCmdData(cmds[0]);
    cmds[0].u_command_type = kLibTvuMediaCtrlCmdInsertKF;
    cmds[0].o_params.o_insertKF.u_program_index = 1;

    // kLibTvuMediaCtrlCmdChangeBitRate
    initCmdData(cmds[1]);
    cmds[1].u_command_type = kLibTvuMediaCtrlCmdChangeBitRate;
    cmds[1].o_params.o_changeBitrate.u_vbitrate = 5000000;
    cmds[1].o_params.o_changeBitrate.u_abitrate = 128000;

    // kLibTvuMediaCtrlCmdChangeResolution
    initCmdData(cmds[2]);
    cmds[2].u_command_type = kLibTvuMediaCtrlCmdChangeResolution;
    cmds[2].o_params.o_changeResolution.u_width = 1920;
    cmds[2].o_params.o_changeResolution.u_height = 1080;

    // kLibTvuMediaCtrlCmdChangeProgramBitRate
    initCmdData(cmds[3]);
    cmds[3].u_command_type = kLibTvuMediaCtrlCmdChangeProgramBitRate;
    cmds[3].o_params.o_progBitrate.u_programIndex = 0;
    cmds[3].o_params.o_progBitrate.u_vbitrate = 8000000;
    cmds[3].o_params.o_progBitrate.u_abitrate = 256000;

    // kLibTvuMediaCtrlCmdChangeAudioParams
    initCmdData(cmds[4]);
    cmds[4].u_command_type = kLibTvuMediaCtrlCmdChangeAudioParams;
    cmds[4].o_params.o_audio.u_audio_only = 0;
    cmds[4].o_params.o_audio.u_aac_profile = 1;
    cmds[4].o_params.o_audio.u_tracks_layout = 0x0003;

    // kLibTvuMediaCtrlCmdChangeDelayParams
    initCmdData(cmds[5]);
    cmds[5].u_command_type = kLibTvuMediaCtrlCmdChangeDelayParams;
    cmds[5].o_params.o_delay.u_delay_ms = 2000;

    // kLibTvuMediaCtrlCmdSendAudioVolumeParams
    initCmdData(cmds[6]);
    cmds[6].u_command_type = kLibTvuMediaCtrlCmdSendAudioVolumeParams;
    cmds[6].o_params.o_audioVolume.u_counts = 2;
    cmds[6].o_params.o_audioVolume.u_volume[0] = 100;
    cmds[6].o_params.o_audioVolume.u_volume[1] = 50;

    // kLibTvuMediaCtrlCmdStartProgLiveParams
    initCmdData(cmds[7]);
    cmds[7].u_command_type = kLibTvuMediaCtrlCmdStartProgLiveParams;
    cmds[7].o_params.o_startLive.u_program_index = 0;
    cmds[7].o_params.o_startLive.b_vbr = 0;
    cmds[7].o_params.o_startLive.u_video_bit_rate = 10000000;

    // kLibTvuMediaCtrlCmdStopProgLiveParams
    initCmdData(cmds[8]);
    cmds[8].u_command_type = kLibTvuMediaCtrlCmdStopProgLiveParams;
    cmds[8].o_params.o_stopLive.u_program_index = 0;

    // kLibTvuMediaCtrlCmdCamerasParams
    initCmdData(cmds[9]);
    cmds[9].u_command_type = kLibTvuMediaCtrlCmdCamerasParams;
    cmds[9].o_params.o_cameras.u_index = 1;
    cmds[9].o_params.o_cameras.u_video_connection = 2;

    const uint8_t* pOut = nullptr;
    int written = LibTvuMediaControlHandleWrite(handle, cmds.data(), 10, &pOut);
    ASSERT_GT(written, 0);

    libtvumedia_control_handle_t readHandle = LibTvuMediaControlHandleCreate();
    const libtvumedia_ctrlcmd_data_t* pParams = nullptr;
    int counts = 0;
    int readResult = LibTvuMediaControlHandleRead(readHandle, pOut, written,
                                                   &pParams, &counts);
    ASSERT_GT(readResult, 0);
    ASSERT_EQ(counts, 10);

    // Verify each command type
    EXPECT_EQ(pParams[0].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdInsertKF);
    EXPECT_EQ(pParams[1].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeBitRate);
    EXPECT_EQ(pParams[2].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeResolution);
    EXPECT_EQ(pParams[3].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeProgramBitRate);
    EXPECT_EQ(pParams[4].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeAudioParams);
    EXPECT_EQ(pParams[5].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdChangeDelayParams);
    EXPECT_EQ(pParams[6].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdSendAudioVolumeParams);
    EXPECT_EQ(pParams[7].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdStartProgLiveParams);
    EXPECT_EQ(pParams[8].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdStopProgLiveParams);
    EXPECT_EQ(pParams[9].u_command_type, (uint32_t)kLibTvuMediaCtrlCmdCamerasParams);

    LibTvuMediaControlHandleDestory(readHandle);
}
