#pragma once
#include "mua2_roster_lifecycle.hpp"

namespace moderngekko::roster {
// Reviewed CCharacters accesses: lookup, definition load, reset, allocation and
// release. String interning, standalone lookup tables, bitmap word steps and NPC
// capacities are deliberately excluded. This is not a standalone install.
inline constexpr std::array ManagerConsumerMembers{
  ManagerMemberSite{0x800b2d08u, 0x806353a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80098b44u, 0x386353acu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80098b5cu, 0x388453acu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80098b6cu, 0x90a37784u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80098b70u, 0x38e353acu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80098b7cu, 0x90837780u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80098ba8u, 0x80637784u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099874u, 0x80632c9cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x800998a0u, 0x38032c9cu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x80099920u, 0x80632d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009992cu, 0x80642c9cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009995cu, 0x38032c9cu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800999d0u, 0x80042d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099a24u, 0x3bbd5f70u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099a30u, 0x801e2d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099a3cu, 0x80632d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099a78u, 0x801f622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x80099accu, 0x3bff5f70u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099ad8u, 0x801a2d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099ae4u, 0x80632d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099b30u, 0x801b622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x80099b8cu, 0x3b9c5f70u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x80099b98u, 0x801d2d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099ba4u, 0x80632d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x80099bf4u, 0x801e622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009a180u, 0x88852d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a190u, 0x98052d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a19cu, 0x80035f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009a1acu, 0x38635cb0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009a1b8u, 0x80645f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009a1c0u, 0x90045f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009a220u, 0x88a62d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a230u, 0x98062d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a238u, 0x80665f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009a25cu, 0x38635cb0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009a260u, 0x38845cb0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009a26cu, 0x80645f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009a274u, 0x90045f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009a2ccu, 0x3bbd2d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a2f0u, 0x801e2d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a328u, 0x801f53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a360u, 0x38c65cb0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009a37cu, 0x88042d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a384u, 0x98042d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a388u, 0x80035f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009a398u, 0x90035f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009a3b4u, 0x88642d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a3c0u, 0x80042d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a3d0u, 0x98042d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a458u, 0x80bf2d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a464u, 0x80042d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a4c4u, 0x80642c9cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a4ccu, 0x90042c9cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a4d0u, 0x80852c90u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a4d4u, 0x80652c98u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a4dcu, 0x90852c90u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a4e8u, 0x90052c98u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a4f4u, 0x90052c90u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a514u, 0x38632be0u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a528u, 0x80862ca4u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a530u, 0x80062d5cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a53cu, 0x90862ca4u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a544u, 0x80852ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a54cu, 0x90052ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a554u, 0x93c52ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a558u, 0x80062d5cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a564u, 0x90052ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a56cu, 0x901f2d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a570u, 0x881f2d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a578u, 0x981f2d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a5b4u, 0x3bbf2d7cu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a5e0u, 0x80032d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a61cu, 0x801f53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a684u, 0x3bde2d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a6f8u, 0x801f53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a734u, 0x80c42d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a740u, 0x80052d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a760u, 0x88642d74u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a920u, 0x807e53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009a930u, 0x3ba32d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009a944u, 0x801e2d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ac28u, 0x806353a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ac54u, 0x90032ca4u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ac60u, 0x38632c9cu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009ade8u, 0x93be2c90u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009adf0u, 0x93be2c94u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009adf4u, 0x93be2c98u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ae00u, 0x809e2c90u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ae04u, 0x807e2c98u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ae0cu, 0x909e2c90u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ae18u, 0x901e2c98u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ae20u, 0x93be2c90u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ae40u, 0x38632be0u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009ae78u, 0x80042d5cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ae7cu, 0x80a62ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ae88u, 0x90062ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ae90u, 0x90e62ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ae94u, 0x80042d5cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009aea0u, 0x90062ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009aea8u, 0x80042d5cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009aeb4u, 0x80a62ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009aec0u, 0x90062ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009aec8u, 0x90e62ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009aeccu, 0x80042d5cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009aed8u, 0x90062ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009aee0u, 0x80042d5cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009aeecu, 0x80a62ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009aef8u, 0x90062ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af00u, 0x90e62ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af04u, 0x80042d5cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af10u, 0x90062ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af18u, 0x80042d5cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af24u, 0x80a62ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af30u, 0x90062ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af38u, 0x90e62ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af3cu, 0x80042d5cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af48u, 0x90062ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af60u, 0x93c353a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009af64u, 0x386353acu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009af70u, 0x386374c4u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009af7cu, 0x93c3622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009af80u, 0x93c35f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009b070u, 0x80632ca4u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b0dcu, 0x800453a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b0f0u, 0x806453a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b0f8u, 0x906453a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b11cu, 0x90192d60u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b2e4u, 0x90192d60u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b2e8u, 0x9ab92d78u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b2f8u, 0x92d92d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b300u, 0x9ad92d74u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b304u, 0x9ad92d75u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b308u, 0x88192d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b310u, 0xb2d92d76u, 65536u, 131072u, 2u},
  ManagerMemberSite{0x8009b318u, 0x98192d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b518u, 0x8005748cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b524u, 0x82c46f10u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b53cu, 0x80647494u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b544u, 0x90047494u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b548u, 0x8065748cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b550u, 0x9005748cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b560u, 0x9005748cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b56cu, 0x80c77490u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b574u, 0x806774c0u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b580u, 0x90c77490u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b58cu, 0x90c774c0u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b590u, 0x386353c4u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b5a0u, 0x92c353b0u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b5b4u, 0x900653bcu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b5bcu, 0x386353acu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b5c0u, 0x900653c0u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b5c8u, 0x388453acu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b5ccu, 0x900653b8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b5dcu, 0x800653acu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b5ecu, 0x800353b8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b5f4u, 0x900353b8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b5f8u, 0x800653acu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b604u, 0x800353b8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b614u, 0x900353b8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b618u, 0x800653b0u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b624u, 0x386374c4u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b654u, 0x38032d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b670u, 0x80042d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b67cu, 0x80032ca4u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b688u, 0x88042d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b698u, 0x98042d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b69cu, 0x80032c94u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b6acu, 0x82e42be0u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b6c8u, 0x80042c9cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b6d4u, 0x90042c9cu, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b6d8u, 0x80832c94u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b6e0u, 0x90032c94u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b6f0u, 0x90032c94u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b6fcu, 0x80662c98u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b700u, 0x80862ca4u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b710u, 0x90a62c98u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b718u, 0x90062ca4u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b744u, 0x80032ca8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b748u, 0x901b2d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b750u, 0x80052d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b75cu, 0x80632d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b774u, 0x80052d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b780u, 0x80632d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b7b8u, 0x80632d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b7c4u, 0x80042d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b88cu, 0x38c35f70u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009b898u, 0x88042d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b8a4u, 0x98042d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009b8a8u, 0x8003622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009b8c8u, 0x8064622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009b8ecu, 0x38635f70u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009b8f0u, 0x38845f70u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009b900u, 0x8085622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009b908u, 0x9005622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009b95cu, 0x800853a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b980u, 0x806853a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009b988u, 0x900853a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009bb8cu, 0x8003622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009bb98u, 0x38635f70u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009bba4u, 0x8064622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009bbacu, 0x9004622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009c360u, 0x39ee2d7cu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009c380u, 0x800e2d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009c3a0u, 0x800e53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ce24u, 0x88032d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009dad4u, 0x80632d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009dae0u, 0x80042d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009db50u, 0x80632d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009db5cu, 0x80042d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009dbb0u, 0x807f53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009dbc0u, 0x3bc32d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009dbd4u, 0x801f2d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009e448u, 0x3bde2d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009e46cu, 0x80032d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009e4f8u, 0x80032d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009e564u, 0x801a53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009e5b8u, 0x3b7b2d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009e5dcu, 0x80032d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009e6f0u, 0x801d53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009e748u, 0x3bff2d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009e76cu, 0x80032d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009e7f4u, 0x801a53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009e864u, 0x3bff2d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009e888u, 0x80032d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009e96cu, 0x801a53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009e9e4u, 0x3ad62d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009ea10u, 0x80032d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ed38u, 0x801f53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009ed84u, 0x3bde2d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x8009eda8u, 0x80032d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009eddcu, 0x801f53a8u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x8009f098u, 0x800354f8u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f0a4u, 0x386345e0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009f0b8u, 0x386345f0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009f0c4u, 0x80075ca4u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f0d0u, 0x80075c98u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f0e0u, 0x81045c60u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f0f8u, 0x80045ca0u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f100u, 0x90045ca0u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f104u, 0x80875c98u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f10cu, 0x90075c98u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f11cu, 0x90075c98u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f128u, 0x80855c9cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f12cu, 0x80655ca4u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f138u, 0x37805510u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009f13cu, 0x90855c9cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f144u, 0x90055ca4u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f198u, 0x80045500u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f1a0u, 0x90045500u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f7ccu, 0x380645f0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009f808u, 0x800354f0u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f838u, 0x386345e0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009f848u, 0x386345e0u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009f86cu, 0x38075510u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009f8a4u, 0x80045ca0u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f8d4u, 0x38065510u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009f908u, 0x80645500u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f914u, 0x90045500u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f930u, 0x80645ca0u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f938u, 0x90045ca0u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f93cu, 0x80865c94u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f940u, 0x80665c9cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f948u, 0x90865c94u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f954u, 0x90065c9cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f960u, 0x90065c94u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f980u, 0x38635c60u, 131072u, 262144u, 1u},
  ManagerMemberSite{0x8009f994u, 0x80855ca4u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x8009f99cu, 0x90055ca4u, 131072u, 262144u, 4u},
  ManagerMemberSite{0x800b2d14u, 0x8063622cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x800b2d28u, 0xa8635f70u, 131072u, 262144u, 2u},
  ManagerMemberSite{0x800b2d34u, 0x80635f6cu, 131072u, 262144u, 4u},
  ManagerMemberSite{0x800b2d48u, 0xa8635cb0u, 131072u, 262144u, 2u},
  ManagerMemberSite{0x800b2d5cu, 0x38632d60u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2d70u, 0x38632d64u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2d84u, 0x38632d68u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2d98u, 0x38632d6cu, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2dacu, 0x88632d75u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2dbcu, 0x80052d58u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x800b2dc4u, 0x80842d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x800b2de8u, 0x80632d70u, 65536u, 131072u, 4u},
  ManagerMemberSite{0x800b2e08u, 0x88032d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2e20u, 0x88032d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2e38u, 0x88032d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2e50u, 0x88032d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2e68u, 0x88032d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2e80u, 0xa8632d76u, 65536u, 131072u, 2u},
  ManagerMemberSite{0x800b2e94u, 0x88032d79u, 65536u, 131072u, 1u},
  ManagerMemberSite{0x800b2e9cu, 0x98032d79u, 65536u, 131072u, 1u}
};
inline constexpr std::array AuxiliaryConsumerFields{
  ConstructorField{0x8009f2a0u, 0x80030f08u, 3848u},
  ConstructorField{0x8009f2acu, 0x83e50e80u, 3712u},
  ConstructorField{0x8009f2c4u, 0x80850f10u, 3856u},
  ConstructorField{0x8009f2ccu, 0x90050f10u, 3856u},
  ConstructorField{0x8009f2d0u, 0x80830f08u, 3848u},
  ConstructorField{0x8009f2d8u, 0x90030f08u, 3848u},
  ConstructorField{0x8009f2e8u, 0x90030f08u, 3848u},
  ConstructorField{0x8009f2f0u, 0x80a30f0cu, 3852u},
  ConstructorField{0x8009f2f4u, 0x80830f18u, 3864u},
  ConstructorField{0x8009f2fcu, 0x90a30f0cu, 3852u},
  ConstructorField{0x8009f304u, 0x90830f18u, 3864u},
  ConstructorField{0x8009fa54u, 0x80640f10u, 3856u},
  ConstructorField{0x8009fa5cu, 0x90040f10u, 3856u},
  ConstructorField{0x8009fa60u, 0x809f0f04u, 3844u},
  ConstructorField{0x8009fa64u, 0x807f0f0cu, 3852u},
  ConstructorField{0x8009fa6cu, 0x909f0f04u, 3844u},
  ConstructorField{0x8009fa78u, 0x901f0f0cu, 3852u},
  ConstructorField{0x8009fa84u, 0x901f0f04u, 3844u},
  ConstructorField{0x8009faa4u, 0x38630e80u, 3712u},
  ConstructorField{0x8009faacu, 0x807f0f18u, 3864u},
  ConstructorField{0x8009fab4u, 0x901f0f18u, 3864u}
};
inline std::optional<std::vector<InstructionPatch>> ConsumerPatches() {
  std::vector<InstructionPatch> patches;
  patches.push_back({0x800b2d04u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  for (const auto& site : ManagerConsumerMembers) {
    const auto old_member = site.old_bias + std::int16_t(site.original & 0xffff);
    const auto member = RelocateMember(Members, old_member, site.width);
    if (!member) return {};
    const auto word = SignedImmediate(site.original, std::int64_t(*member) - site.new_bias);
    if (!word) return {};
    patches.push_back({site.address, site.original, *word});
  }
  for (const auto& site : AuxiliaryConsumerFields) {
    const auto member = RelocateMember(AuxiliaryRegions, site.member, 4);
    if (!member) return {};
    const auto word = SignedImmediate(site.original, *member);
    if (!word) return {};
    patches.push_back({site.address, site.original, *word});
  }
  patches.push_back({0x80098b3cu, 0x3c7b0001u, *SignedImmediate(0x3c7b0001u, 2)});
  patches.push_back({0x80098b50u, 0x3c9b0001u, *SignedImmediate(0x3c9b0001u, 2)});
  patches.push_back({0x80098b64u, 0x3c7b0001u, *SignedImmediate(0x3c7b0001u, 2)});
  patches.push_back({0x80098ba0u, 0x3c7b0001u, *SignedImmediate(0x3c7b0001u, 2)});
  patches.push_back({0x8009986cu, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x8009989cu, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x80099918u, 0x3c9f0001u, *SignedImmediate(0x3c9f0001u, 2)});
  patches.push_back({0x80099958u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x80099a0cu, 0x3fa30002u, *SignedImmediate(0x3fa30002u, 4)});
  patches.push_back({0x80099a18u, 0x3fc30001u, *SignedImmediate(0x3fc30001u, 2)});
  patches.push_back({0x80099ab4u, 0x3fe30002u, *SignedImmediate(0x3fe30002u, 4)});
  patches.push_back({0x80099ac0u, 0x3f430001u, *SignedImmediate(0x3f430001u, 2)});
  patches.push_back({0x80099b74u, 0x3f830002u, *SignedImmediate(0x3f830002u, 4)});
  patches.push_back({0x80099b80u, 0x3fa30001u, *SignedImmediate(0x3fa30001u, 2)});
  patches.push_back({0x8009a178u, 0x3c830001u, *SignedImmediate(0x3c830001u, 2)});
  patches.push_back({0x8009a194u, 0x3c630002u, *SignedImmediate(0x3c630002u, 4)});
  patches.push_back({0x8009a1b4u, 0x3c9f0002u, *SignedImmediate(0x3c9f0002u, 4)});
  patches.push_back({0x8009a218u, 0x3ca30001u, *SignedImmediate(0x3ca30001u, 2)});
  patches.push_back({0x8009a234u, 0x3cc30002u, *SignedImmediate(0x3cc30002u, 4)});
  patches.push_back({0x8009a268u, 0x3c9e0002u, *SignedImmediate(0x3c9e0002u, 4)});
  patches.push_back({0x8009a2bcu, 0x3fa30001u, *SignedImmediate(0x3fa30001u, 2)});
  patches.push_back({0x8009a2e0u, 0x3fda0001u, *SignedImmediate(0x3fda0001u, 2)});
  patches.push_back({0x8009a350u, 0x3cc30002u, *SignedImmediate(0x3cc30002u, 4)});
  patches.push_back({0x8009a354u, 0x3ca30001u, *SignedImmediate(0x3ca30001u, 2)});
  patches.push_back({0x8009a3acu, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x8009a440u, 0x3c830001u, *SignedImmediate(0x3c830001u, 2)});
  patches.push_back({0x8009a498u, 0x3cbd0001u, *SignedImmediate(0x3cbd0001u, 2)});
  patches.push_back({0x8009a504u, 0x3c7d0001u, *SignedImmediate(0x3c7d0001u, 2)});
  patches.push_back({0x8009a520u, 0x3cdd0001u, *SignedImmediate(0x3cdd0001u, 2)});
  patches.push_back({0x8009a5acu, 0x3fe30001u, *SignedImmediate(0x3fe30001u, 2)});
  patches.push_back({0x8009a5d8u, 0x3c7b0001u, *SignedImmediate(0x3c7b0001u, 2)});
  patches.push_back({0x8009a678u, 0x3fdb0001u, *SignedImmediate(0x3fdb0001u, 2)});
  patches.push_back({0x8009a724u, 0x3ca30001u, *SignedImmediate(0x3ca30001u, 2)});
  patches.push_back({0x8009a918u, 0x3fc30001u, *SignedImmediate(0x3fc30001u, 2)});
  patches.push_back({0x8009ac24u, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x8009ac4cu, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x8009ade0u, 0x3fdf0001u, *SignedImmediate(0x3fdf0001u, 2)});
  patches.push_back({0x8009ae64u, 0x3c9f0001u, *SignedImmediate(0x3c9f0001u, 2)});
  patches.push_back({0x8009ae74u, 0x3cc80001u, *SignedImmediate(0x3cc80001u, 2)});
  patches.push_back({0x8009aeacu, 0x3cc80001u, *SignedImmediate(0x3cc80001u, 2)});
  patches.push_back({0x8009aee4u, 0x3cc80001u, *SignedImmediate(0x3cc80001u, 2)});
  patches.push_back({0x8009af1cu, 0x3cc80001u, *SignedImmediate(0x3cc80001u, 2)});
  patches.push_back({0x8009af58u, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x8009af6cu, 0x3c7f0001u, *SignedImmediate(0x3c7f0001u, 2)});
  patches.push_back({0x8009af78u, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x8009b06cu, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x8009b0d4u, 0x3c9c0001u, *SignedImmediate(0x3c9c0001u, 2)});
  patches.push_back({0x8009b510u, 0x3cbc0001u, *SignedImmediate(0x3cbc0001u, 2)});
  patches.push_back({0x8009b564u, 0x3cfc0001u, *SignedImmediate(0x3cfc0001u, 2)});
  patches.push_back({0x8009b59cu, 0x3c7c0001u, *SignedImmediate(0x3c7c0001u, 2)});
  patches.push_back({0x8009b5d4u, 0x3cdc0001u, *SignedImmediate(0x3cdc0001u, 2)});
  patches.push_back({0x8009b64cu, 0x3c7c0001u, *SignedImmediate(0x3c7c0001u, 2)});
  patches.push_back({0x8009b668u, 0x3c7c0001u, *SignedImmediate(0x3c7c0001u, 2)});
  patches.push_back({0x8009b6f8u, 0x3cdc0001u, *SignedImmediate(0x3cdc0001u, 2)});
  patches.push_back({0x8009b728u, 0x3cbc0001u, *SignedImmediate(0x3cbc0001u, 2)});
  patches.push_back({0x8009b7a8u, 0x3c9c0001u, *SignedImmediate(0x3c9c0001u, 2)});
  patches.push_back({0x8009b880u, 0x3c7f0002u, *SignedImmediate(0x3c7f0002u, 4)});
  patches.push_back({0x8009b888u, 0x3c9f0001u, *SignedImmediate(0x3c9f0001u, 2)});
  patches.push_back({0x8009b8c4u, 0x3c9f0002u, *SignedImmediate(0x3c9f0002u, 4)});
  patches.push_back({0x8009b8f8u, 0x3cbf0002u, *SignedImmediate(0x3cbf0002u, 4)});
  patches.push_back({0x8009b954u, 0x3d030001u, *SignedImmediate(0x3d030001u, 2)});
  patches.push_back({0x8009bb80u, 0x3c770002u, *SignedImmediate(0x3c770002u, 4)});
  patches.push_back({0x8009bba0u, 0x3c970002u, *SignedImmediate(0x3c970002u, 4)});
  patches.push_back({0x8009c358u, 0x3dd70001u, *SignedImmediate(0x3dd70001u, 2)});
  patches.push_back({0x8009ce1cu, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x8009daccu, 0x3c9e0001u, *SignedImmediate(0x3c9e0001u, 2)});
  patches.push_back({0x8009db48u, 0x3c9f0001u, *SignedImmediate(0x3c9f0001u, 2)});
  patches.push_back({0x8009db9cu, 0x3fe30001u, *SignedImmediate(0x3fe30001u, 2)});
  patches.push_back({0x8009e42cu, 0x3fc30001u, *SignedImmediate(0x3fc30001u, 2)});
  patches.push_back({0x8009e468u, 0x3c7b0001u, *SignedImmediate(0x3c7b0001u, 2)});
  patches.push_back({0x8009e4f4u, 0x3c7b0001u, *SignedImmediate(0x3c7b0001u, 2)});
  patches.push_back({0x8009e5a4u, 0x3f630001u, *SignedImmediate(0x3f630001u, 2)});
  patches.push_back({0x8009e5d8u, 0x3c7e0001u, *SignedImmediate(0x3c7e0001u, 2)});
  patches.push_back({0x8009e730u, 0x3fe30001u, *SignedImmediate(0x3fe30001u, 2)});
  patches.push_back({0x8009e768u, 0x3c7b0001u, *SignedImmediate(0x3c7b0001u, 2)});
  patches.push_back({0x8009e848u, 0x3fe30001u, *SignedImmediate(0x3fe30001u, 2)});
  patches.push_back({0x8009e884u, 0x3c7b0001u, *SignedImmediate(0x3c7b0001u, 2)});
  patches.push_back({0x8009e9acu, 0x3ec30001u, *SignedImmediate(0x3ec30001u, 2)});
  patches.push_back({0x8009ea0cu, 0x3c6f0001u, *SignedImmediate(0x3c6f0001u, 2)});
  patches.push_back({0x8009ed70u, 0x3fc30001u, *SignedImmediate(0x3fc30001u, 2)});
  patches.push_back({0x8009eda4u, 0x3c7b0001u, *SignedImmediate(0x3c7b0001u, 2)});
  patches.push_back({0x8009f094u, 0x3c630002u, *SignedImmediate(0x3c630002u, 4)});
  patches.push_back({0x8009f0b0u, 0x3c7d0002u, *SignedImmediate(0x3c7d0002u, 4)});
  patches.push_back({0x8009f0c0u, 0x3ce30002u, *SignedImmediate(0x3ce30002u, 4)});
  patches.push_back({0x8009f124u, 0x3ca30002u, *SignedImmediate(0x3ca30002u, 4)});
  patches.push_back({0x8009f18cu, 0x3c1d0002u, *SignedImmediate(0x3c1d0002u, 4)});
  patches.push_back({0x8009f7c4u, 0x3cde0002u, *SignedImmediate(0x3cde0002u, 4)});
  patches.push_back({0x8009f830u, 0x3c7e0002u, *SignedImmediate(0x3c7e0002u, 4)});
  patches.push_back({0x8009f844u, 0x3c7e0002u, *SignedImmediate(0x3c7e0002u, 4)});
  patches.push_back({0x8009f864u, 0x3cfe0002u, *SignedImmediate(0x3cfe0002u, 4)});
  patches.push_back({0x8009f8ccu, 0x3cde0002u, *SignedImmediate(0x3cde0002u, 4)});
  patches.push_back({0x8009f970u, 0x3c7e0002u, *SignedImmediate(0x3c7e0002u, 4)});
  patches.push_back({0x8009f98cu, 0x3cbe0002u, *SignedImmediate(0x3cbe0002u, 4)});
  patches.push_back({0x800b2d10u, 0x3c630002u, *SignedImmediate(0x3c630002u, 4)});
  patches.push_back({0x800b2d1cu, 0x3c630002u, *SignedImmediate(0x3c630002u, 4)});
  patches.push_back({0x800b2d30u, 0x3c630002u, *SignedImmediate(0x3c630002u, 4)});
  patches.push_back({0x800b2d3cu, 0x3c630002u, *SignedImmediate(0x3c630002u, 4)});
  patches.push_back({0x800b2d54u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2d68u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2d7cu, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2d90u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2da4u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2db8u, 0x3ca30001u, *SignedImmediate(0x3ca30001u, 2)});
  patches.push_back({0x800b2de0u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2e00u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2e18u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2e30u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2e48u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2e60u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2e78u, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  patches.push_back({0x800b2e8cu, 0x3c630001u, *SignedImmediate(0x3c630001u, 2)});
  // Bounds, including 16 reset groups of four generation handles.
  patches.push_back({0x80099884u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009988cu, 0x3800002cu, *SignedImmediate(0x3800002cu, Expanded.stats.capacity)});
  patches.push_back({0x800998f4u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x800998f8u, 0x3800002cu, *SignedImmediate(0x3800002cu, Expanded.stats.capacity)});
  patches.push_back({0x80099940u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x80099948u, 0x3800002cu, *SignedImmediate(0x3800002cu, Expanded.stats.capacity)});
  patches.push_back({0x800999b0u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x800999b4u, 0x3800002cu, *SignedImmediate(0x3800002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009a4e0u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009a4f8u, 0x3800002bu, *SignedImmediate(0x3800002bu, Expanded.stats.capacity - 1)});
  patches.push_back({0x8009ac80u, 0x2c03002cu, *SignedImmediate(0x2c03002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009ac88u, 0x3ba0002cu, *SignedImmediate(0x3ba0002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009ace8u, 0x2c03002cu, *SignedImmediate(0x2c03002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009acecu, 0x3ba0002cu, *SignedImmediate(0x3ba0002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009acf8u, 0x2c1d002cu, *SignedImmediate(0x2c1d002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009ad20u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009ad28u, 0x3ba0002cu, *SignedImmediate(0x3ba0002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009ad50u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009ad58u, 0x3ba0002cu, *SignedImmediate(0x3ba0002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009adb8u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009adbcu, 0x3ba0002cu, *SignedImmediate(0x3ba0002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009adc8u, 0x2c1d002cu, *SignedImmediate(0x2c1d002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009ae10u, 0x2c04002cu, *SignedImmediate(0x2c04002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009ae24u, 0x3800002bu, *SignedImmediate(0x3800002bu, Expanded.stats.capacity - 1)});
  patches.push_back({0x8009ae54u, 0x2c00002cu, *SignedImmediate(0x2c00002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009ae5cu, 0x3800000bu, *SignedImmediate(0x3800000bu, Expanded.stats.capacity / 4)});
  patches.push_back({0x8009b680u, 0x2c00002cu, *SignedImmediate(0x2c00002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009b6e4u, 0x2c00002cu, *SignedImmediate(0x2c00002cu, Expanded.stats.capacity)});
  patches.push_back({0x8009f09cu, 0x2c000021u, *SignedImmediate(0x2c000021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x8009f2dcu, 0x2c000021u, *SignedImmediate(0x2c000021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x8009fa70u, 0x2c040021u, *SignedImmediate(0x2c040021u, Expanded.auxiliary.capacity)});
  patches.push_back({0x8009fa88u, 0x38000020u, *SignedImmediate(0x38000020u, Expanded.auxiliary.capacity - 1)});
  std::sort(patches.begin(), patches.end(), [](const auto& a, const auto& b) {
    return a.address < b.address;
  });
  return patches;
}
} // namespace moderngekko::roster
