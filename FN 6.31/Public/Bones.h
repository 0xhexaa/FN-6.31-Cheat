#pragma once

namespace Bones {
    // Spine chain
    const int32_t Pelvis = 2;      // Base of spine
    const int32_t Spine_01 = 3;    // Lower back
    const int32_t Spine_02 = 4;    // Mid back
    const int32_t Spine_03 = 5;    // Upper back
    const int32_t Spine_04 = 6;    // Lower chest
    const int32_t Spine_05 = 7;    // Upper chest

    // Head
    const int32_t Neck_01 = 64;    // Base of neck
    const int32_t Neck_02 = 65;    // Mid neck
    const int32_t Head = 66;       // Top of head

    // Left arm (positive X values)
    const int32_t Clavicle_l = 9;  // Left shoulder
    const int32_t Upperarm_l = 10; // Left upper arm
    const int32_t Lowerarm_l = 11; // Left forearm
    const int32_t Hand_l = 12;     // Left hand

    // Right arm (negative X values)
    const int32_t Clavicle_r = 37; // Right shoulder
    const int32_t Upperarm_r = 38; // Right upper arm
    const int32_t Lowerarm_r = 39; // Right forearm
    const int32_t Hand_r = 40;     // Right hand

    // Left leg (positive X values)
    const int32_t Thigh_l = 73;    // Left thigh
    const int32_t Calf_l = 68;     // Left shin
    const int32_t Foot_l = 69;     // Left foot
    const int32_t Ball_l = 71;     // Left toe

    // Right leg (negative X values)
    const int32_t Thigh_r = 80;    // Right thigh
    const int32_t Calf_r = 75;     // Right shin
    const int32_t Foot_r = 76;     // Right foot
    const int32_t Ball_r = 78;     // Right toe
}