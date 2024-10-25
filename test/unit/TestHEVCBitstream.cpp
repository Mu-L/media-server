#include "TestCommon.h"

#include "h265/h265.h"

TEST(TestH265SeqParameterSet, DISABLED_Parse)
{
        Logger::EnableDebug(true);

        
        const uint8_t data[] = {
                0x01, 0x01, 0x40, 0x00, 0x00, 0x00, 0x90, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x7b, 0xa0, 0x03, 0xc0,
                0x80, 0x11, 0x07, 0xcb, 0x96, 0x5d, 0x29, 0x08,
                0x46, 0x45, 0xff, 0x8c, 0x05, 0xa8, 0x08, 0x08,
                0x08, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
                0x07, 0x8c, 0x00, 0xbb, 0xca, 0x20, 0x00, 0x09,
                0x89, 0x68, 0x00, 0x00, 0x4c, 0x4b, 0x40, 0x80,
        };


        H265SeqParameterSet sps;

        ASSERT_TRUE(sps.Decode(data, sizeof(data)));

        // sps.Dump();
}

TEST(TestH265SeqParameterSet, TestSampleSPS)
{
        Logger::EnableDebug(true);

        
        const uint8_t data[] = {
        0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0xb0,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x5d,
        0xa0, 0x02, 0x80, 0x80, 0x2e, 0x1f, 0x13, 0x96,
        0xbb, 0x93, 0x24, 0xbb, 0x95, 0x82, 0x83, 0x03,
        0x01, 0x76, 0x85, 0x09, 0x40
        };


        H265SeqParameterSet sps;
        bool res = sps.Decode(data, sizeof(data));
        
        ASSERT_TRUE(res);
        EXPECT_EQ(0, sps.vps_id);
        EXPECT_EQ(0, sps.max_sub_layers_minus1);
        EXPECT_EQ(0, sps.ext_or_max_sub_layers_minus1); // currently unsupported
        EXPECT_EQ(1, sps.temporal_id_nesting_flag);

        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.profile_space);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.tier_flag);
        EXPECT_EQ(1, sps.profile_tier_level.general_profile_tier_level.profile_idc);
        EXPECT_THAT(sps.profile_tier_level.general_profile_tier_level.profile_compatibility_flag,
              ::testing::ElementsAreArray({0, 1, 1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0}));
        EXPECT_EQ(1, sps.profile_tier_level.general_profile_tier_level.progressive_source_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.interlaced_source_flag);
        EXPECT_EQ(1, sps.profile_tier_level.general_profile_tier_level.non_packed_constraint_flag);
        EXPECT_EQ(1, sps.profile_tier_level.general_profile_tier_level.frame_only_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.max_12bit_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.max_10bit_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.max_8bit_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.max_422chroma_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.max_420chroma_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.max_monochrome_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.intra_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.one_picture_only_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.lower_bit_rate_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.max_14bit_constraint_flag);
        EXPECT_EQ(0, sps.profile_tier_level.general_profile_tier_level.inbld_flag);
        EXPECT_EQ(93, sps.profile_tier_level.general_profile_tier_level.level_idc);

        EXPECT_EQ(1280, sps.pic_width_in_luma_samples);
        EXPECT_EQ(736, sps.pic_height_in_luma_samples);
        EXPECT_EQ(1, sps.conformance_window_flag);
        
        EXPECT_EQ(0, sps.pic_conf_win.left_offset);
        EXPECT_EQ(0, sps.pic_conf_win.right_offset);
        EXPECT_EQ(0, sps.pic_conf_win.top_offset);
        EXPECT_EQ(16, sps.pic_conf_win.bottom_offset);

        EXPECT_EQ(0, sps.seq_parameter_set_id);
        EXPECT_EQ(1, sps.chroma_format_idc);
        EXPECT_EQ(0, sps.separate_colour_plane_flag);
        EXPECT_EQ(0, sps.bit_depth_luma_minus8);
        EXPECT_EQ(0, sps.bit_depth_chroma_minus8);
        EXPECT_EQ(4, sps.log2_max_pic_order_cnt_lsb_minus4);
        EXPECT_EQ(1, sps.sps_sub_layer_ordering_info_present_flag);
        EXPECT_EQ(0, sps.log2_min_luma_coding_block_size_minus3);
        EXPECT_EQ(2, sps.log2_diff_max_min_luma_coding_block_size);
        EXPECT_EQ(0, sps.log2_min_luma_transform_block_size_minus2);
        EXPECT_EQ(3, sps.log2_diff_max_min_luma_transform_block_size);

        sps.Dump();
}

TEST(TestH265PictureParameterSet, TestSamplePPS)
{
        Logger::EnableDebug(true);

        
        const uint8_t data[] = {
        0xc0, 0xf3, 0xc0, 0x02, 0x10, 0x00
        };


        H265PictureParameterSet pps;
        bool res = pps.Decode(data, sizeof(data));
        
        ASSERT_TRUE(res);
        EXPECT_EQ(0, pps.pps_id);
        EXPECT_EQ(0, pps.sps_id);
        EXPECT_EQ(0, pps.dependent_slice_segments_enabled_flag);
        EXPECT_EQ(0, pps.output_flag_present_flag);
        EXPECT_EQ(0, pps.num_extra_slice_header_bits);
        EXPECT_EQ(0, pps.sign_data_hiding_flag);
        EXPECT_EQ(1, pps.cabac_init_present_flag);
        EXPECT_EQ(0, pps.num_ref_idx_l0_default_active_minus1);
        EXPECT_EQ(0, pps.num_ref_idx_l1_default_active_minus1);

        // pps.Dump();
}

TEST(TestH265VideoParameterSet, TestSampleVPS)
{
        Logger::EnableDebug(true);

        
        const uint8_t data[] = {
        0x0c, 0x01, 0xff, 0xff, 0x01, 0x60, 0x00, 0x00,
        0x03, 0x00, 0xb0, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x03, 0x00, 0x5d, 0xac, 0x59, 0x00
        };

        H265VideoParameterSet vps;
        bool res = vps.Decode(data, sizeof(data));
        
        ASSERT_TRUE(res);
        EXPECT_EQ(0, vps.vps_id);
        EXPECT_EQ(0, vps.vps_max_layers_minus1);
        EXPECT_EQ(0, vps.vps_max_sub_layers_minus1);
        EXPECT_EQ(1, vps.vps_temporal_id_nesting_flag);

        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.profile_space);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.tier_flag);
        EXPECT_EQ(1, vps.profile_tier_level.general_profile_tier_level.profile_idc);
        EXPECT_THAT(vps.profile_tier_level.general_profile_tier_level.profile_compatibility_flag,
              ::testing::ElementsAreArray({0, 1, 1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0}));
        EXPECT_EQ(1, vps.profile_tier_level.general_profile_tier_level.progressive_source_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.interlaced_source_flag);
        EXPECT_EQ(1, vps.profile_tier_level.general_profile_tier_level.non_packed_constraint_flag);
        EXPECT_EQ(1, vps.profile_tier_level.general_profile_tier_level.frame_only_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.max_12bit_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.max_10bit_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.max_8bit_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.max_422chroma_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.max_420chroma_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.max_monochrome_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.intra_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.one_picture_only_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.lower_bit_rate_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.max_14bit_constraint_flag);
        EXPECT_EQ(0, vps.profile_tier_level.general_profile_tier_level.inbld_flag);
        EXPECT_EQ(93, vps.profile_tier_level.general_profile_tier_level.level_idc);

        vps.Dump();
}