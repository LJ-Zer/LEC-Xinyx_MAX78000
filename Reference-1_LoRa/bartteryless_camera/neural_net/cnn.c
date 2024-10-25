/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
*
* This software is protected by copyright laws of the United States and
* of foreign countries. This material may also be protected by patent laws
* and technology transfer regulations of the United States and of foreign
* countries. This software is furnished under a license agreement and/or a
* nondisclosure agreement and may only be used or reproduced in accordance
* with the terms of those agreements. Dissemination of this information to
* any party or parties not specified in the license agreement and/or
* nondisclosure agreement is expressly prohibited.
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************/

// meme_net_presentation
// Created using ./ai8xize.py --verbose --log --test-dir demos --prefix meme_net_presentation --checkpoint-file ./trained/memenet_presentation/meme_net-q.pth.tar --config-file networks/facenet.yaml --device MAX78000 --compact-data --mexpress --softmax --overwrite

// DO NOT EDIT - regenerate this file instead!

// Configuring 8 layers:
// Layer 0: 1x32x32 (CHW data), no pooling, conv2d with kernel size 3x3, stride 1/1, pad 0/0, ReLU, 32x30x30 output
// Layer 1: 32x30x30 (HWC data), no pooling, conv2d with kernel size 3x3, stride 1/1, pad 0/0, ReLU, 24x28x28 output
// Layer 2: 24x28x28 (HWC data), 2x2 max pool with stride 2/2, conv2d with kernel size 3x3, stride 1/1, pad 0/0, ReLU, 24x12x12 output
// Layer 3: 24x12x12 (HWC data), no pooling, conv2d with kernel size 3x3, stride 1/1, pad 0/0, ReLU, 24x10x10 output
// Layer 4: 24x10x10 (HWC data), 2x2 max pool with stride 2/2, no convolution, 24x5x5 output
// Layer 5: 24x5x5 (flattened to 600x1x1, HWC data), no pooling, linear with kernel size 1x1, stride 1/1, pad 0/0, no activation, 32x1x1 output
// Layer 6: 32x1x1 (flattened to 32x1x1, HWC data), no pooling, linear with kernel size 1x1, stride 1/1, pad 0/0, no activation, 16x1x1 output
// Layer 7: 16x1x1 (flattened to 16x1x1, HWC data), no pooling, linear with kernel size 1x1, stride 1/1, pad 0/0, no activation, 4x1x1 output

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "mxc.h"
#include "gcfr_regs.h"
#include "cnn.h"
#include "weights.h"

void CNN_ISR(void)
{
  // Acknowledge interrupt to all groups
  *((volatile uint32_t *) 0x50100000) &= ~((1<<12) | 1);
  *((volatile uint32_t *) 0x50500000) &= ~((1<<12) | 1);

  CNN_COMPLETE; // Signal that processing is complete
#ifdef CNN_INFERENCE_TIMER
  cnn_time = MXC_TMR_SW_Stop(CNN_INFERENCE_TIMER);
#else
  cnn_time = 1;
#endif
}

int cnn_continue(void)
{
  cnn_time = 0;

  *((volatile uint32_t *) 0x50100000) |= 1; // Re-enable group 0

  return CNN_OK;
}

int cnn_stop(void)
{
  *((volatile uint32_t *) 0x50100000) &= ~1; // Disable group 0

  return CNN_OK;
}

void memcpy32(uint32_t *dst, const uint32_t *src, int n)
{
  while (n-- > 0) {
    *dst++ = *src++;
  }
}

// Kernels:
static const uint32_t kernels_0[] = KERNELS_0;
static const uint32_t kernels_1[] = KERNELS_1;
static const uint32_t kernels_2[] = KERNELS_2;
static const uint32_t kernels_3[] = KERNELS_3;
static const uint32_t kernels_4[] = KERNELS_4;
static const uint32_t kernels_5[] = KERNELS_5;
static const uint32_t kernels_6[] = KERNELS_6;
static const uint32_t kernels_7[] = KERNELS_7;
static const uint32_t kernels_8[] = KERNELS_8;
static const uint32_t kernels_9[] = KERNELS_9;
static const uint32_t kernels_10[] = KERNELS_10;
static const uint32_t kernels_11[] = KERNELS_11;
static const uint32_t kernels_12[] = KERNELS_12;
static const uint32_t kernels_13[] = KERNELS_13;
static const uint32_t kernels_14[] = KERNELS_14;
static const uint32_t kernels_15[] = KERNELS_15;
static const uint32_t kernels_16[] = KERNELS_16;
static const uint32_t kernels_17[] = KERNELS_17;
static const uint32_t kernels_18[] = KERNELS_18;
static const uint32_t kernels_19[] = KERNELS_19;
static const uint32_t kernels_20[] = KERNELS_20;
static const uint32_t kernels_21[] = KERNELS_21;
static const uint32_t kernels_22[] = KERNELS_22;
static const uint32_t kernels_23[] = KERNELS_23;
static const uint32_t kernels_24[] = KERNELS_24;
static const uint32_t kernels_25[] = KERNELS_25;
static const uint32_t kernels_26[] = KERNELS_26;
static const uint32_t kernels_27[] = KERNELS_27;
static const uint32_t kernels_28[] = KERNELS_28;
static const uint32_t kernels_29[] = KERNELS_29;
static const uint32_t kernels_30[] = KERNELS_30;
static const uint32_t kernels_31[] = KERNELS_31;

int cnn_load_weights(void)
{
  *((volatile uint8_t *) 0x50180001) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50180000, kernels_0, 453);
  *((volatile uint8_t *) 0x50184081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50184000, kernels_1, 381);
  *((volatile uint8_t *) 0x50188081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50188000, kernels_2, 381);
  *((volatile uint8_t *) 0x5018c081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x5018c000, kernels_3, 381);
  *((volatile uint8_t *) 0x50190081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50190000, kernels_4, 381);
  *((volatile uint8_t *) 0x50194081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50194000, kernels_5, 381);
  *((volatile uint8_t *) 0x50198081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50198000, kernels_6, 381);
  *((volatile uint8_t *) 0x5019c081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x5019c000, kernels_7, 381);
  *((volatile uint8_t *) 0x501a0081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x501a0000, kernels_8, 381);
  *((volatile uint8_t *) 0x501a4081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x501a4000, kernels_9, 381);
  *((volatile uint8_t *) 0x501a8081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x501a8000, kernels_10, 381);
  *((volatile uint8_t *) 0x501ac081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x501ac000, kernels_11, 381);
  *((volatile uint8_t *) 0x501b0081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x501b0000, kernels_12, 381);
  *((volatile uint8_t *) 0x501b4081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x501b4000, kernels_13, 381);
  *((volatile uint8_t *) 0x501b8081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x501b8000, kernels_14, 381);
  *((volatile uint8_t *) 0x501bc081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x501bc000, kernels_15, 381);
  *((volatile uint8_t *) 0x50580081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50580000, kernels_16, 374);
  *((volatile uint8_t *) 0x50584081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50584000, kernels_17, 374);
  *((volatile uint8_t *) 0x50588081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50588000, kernels_18, 374);
  *((volatile uint8_t *) 0x5058c081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x5058c000, kernels_19, 374);
  *((volatile uint8_t *) 0x50590081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50590000, kernels_20, 374);
  *((volatile uint8_t *) 0x50594081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50594000, kernels_21, 374);
  *((volatile uint8_t *) 0x50598081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x50598000, kernels_22, 374);
  *((volatile uint8_t *) 0x5059c081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x5059c000, kernels_23, 374);
  *((volatile uint8_t *) 0x505a0081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x505a0000, kernels_24, 374);
  *((volatile uint8_t *) 0x505a4081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x505a4000, kernels_25, 374);
  *((volatile uint8_t *) 0x505a8081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x505a8000, kernels_26, 374);
  *((volatile uint8_t *) 0x505ac081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x505ac000, kernels_27, 374);
  *((volatile uint8_t *) 0x505b0081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x505b0000, kernels_28, 374);
  *((volatile uint8_t *) 0x505b4081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x505b4000, kernels_29, 374);
  *((volatile uint8_t *) 0x505b8081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x505b8000, kernels_30, 374);
  *((volatile uint8_t *) 0x505bc081) = 0x01; // Set address
  memcpy32((uint32_t *) 0x505bc000, kernels_31, 374);

  return CNN_OK;
}

static const uint8_t bias_0[] = BIAS_0;
static const uint8_t bias_1[] = BIAS_1;

static void memcpy_8to32(uint32_t *dst, const uint8_t *src, int n)
{
  while (n-- > 0) {
    *dst++ = *src++;
  }
}

int cnn_load_bias(void)
{
  memcpy_8to32((uint32_t *) 0x50108000, bias_0, sizeof(uint8_t) * 32);
  memcpy_8to32((uint32_t *) 0x50508000, bias_1, sizeof(uint8_t) * 20);

  return CNN_OK;
}

int cnn_init(void)
{
  *((volatile uint32_t *) 0x50001000) = 0x00000000; // AON control
  *((volatile uint32_t *) 0x50100000) = 0x00100008; // Stop SM
  *((volatile uint32_t *) 0x50100004) = 0x0000040e; // SRAM control
  *((volatile uint32_t *) 0x50100008) = 0x00000007; // Layer count
  *((volatile uint32_t *) 0x50500000) = 0x00100008; // Stop SM
  *((volatile uint32_t *) 0x50500004) = 0x0000040e; // SRAM control
  *((volatile uint32_t *) 0x50500008) = 0x00000007; // Layer count

  return CNN_OK;
}

int cnn_configure(void)
{
  // Layer 0 group 0
  *((volatile uint32_t *) 0x50100010) = 0x0000001f; // Rows
  *((volatile uint32_t *) 0x50100090) = 0x0000001f; // Columns
  *((volatile uint32_t *) 0x50100310) = 0x00000800; // SRAM write ptr
  *((volatile uint32_t *) 0x50100410) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x50100590) = 0x00000b60; // Layer control
  *((volatile uint32_t *) 0x50100a10) = 0x0000f800; // Layer control 2
  *((volatile uint32_t *) 0x50100610) = 0x000000f8; // Mask offset and count
  *((volatile uint32_t *) 0x50100690) = 0x0000001d; // TRAM ptr max
  *((volatile uint32_t *) 0x50100790) = 0x00022000; // Post processing register
  *((volatile uint32_t *) 0x50100710) = 0x00010001; // Mask and processor enables

  // Layer 0 group 1
  *((volatile uint32_t *) 0x50500010) = 0x0000001f; // Rows
  *((volatile uint32_t *) 0x50500090) = 0x0000001f; // Columns
  *((volatile uint32_t *) 0x50500310) = 0x00000800; // SRAM write ptr
  *((volatile uint32_t *) 0x50500410) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x50500590) = 0x00000b60; // Layer control
  *((volatile uint32_t *) 0x50500a10) = 0x0000f800; // Layer control 2
  *((volatile uint32_t *) 0x50500610) = 0x000000f8; // Mask offset and count
  *((volatile uint32_t *) 0x50500690) = 0x0000001d; // TRAM ptr max
  *((volatile uint32_t *) 0x50500790) = 0x00022000; // Post processing register

  // Layer 1 group 0
  *((volatile uint32_t *) 0x50100014) = 0x0000001d; // Rows
  *((volatile uint32_t *) 0x50100094) = 0x0000001d; // Columns
  *((volatile uint32_t *) 0x50100414) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x50100514) = 0x00000800; // SRAM read ptr
  *((volatile uint32_t *) 0x50100594) = 0x00002b20; // Layer control
  *((volatile uint32_t *) 0x50100a14) = 0x0000b800; // Layer control 2
  *((volatile uint32_t *) 0x50100614) = 0x010001b8; // Mask offset and count
  *((volatile uint32_t *) 0x50100694) = 0x0000001b; // TRAM ptr max
  *((volatile uint32_t *) 0x50100714) = 0xffffffff; // Mask and processor enables

  // Layer 1 group 1
  *((volatile uint32_t *) 0x50500014) = 0x0000001d; // Rows
  *((volatile uint32_t *) 0x50500094) = 0x0000001d; // Columns
  *((volatile uint32_t *) 0x50500414) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x50500514) = 0x00000800; // SRAM read ptr
  *((volatile uint32_t *) 0x50500594) = 0x00000b20; // Layer control
  *((volatile uint32_t *) 0x50500a14) = 0x0000b800; // Layer control 2
  *((volatile uint32_t *) 0x50500614) = 0x010001b8; // Mask offset and count
  *((volatile uint32_t *) 0x50500694) = 0x0000001b; // TRAM ptr max
  *((volatile uint32_t *) 0x50500714) = 0xffffffff; // Mask and processor enables

  // Layer 2 group 0
  *((volatile uint32_t *) 0x50100018) = 0x0000001b; // Rows
  *((volatile uint32_t *) 0x50100098) = 0x0000001b; // Columns
  *((volatile uint32_t *) 0x50100198) = 0x00000001; // Pooling rows
  *((volatile uint32_t *) 0x50100218) = 0x00000001; // Pooling columns
  *((volatile uint32_t *) 0x50100298) = 0x00000001; // Stride
  *((volatile uint32_t *) 0x50100318) = 0x00000800; // SRAM write ptr
  *((volatile uint32_t *) 0x50100418) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x50100598) = 0x00002ba0; // Layer control
  *((volatile uint32_t *) 0x50100a18) = 0x0000b800; // Layer control 2
  *((volatile uint32_t *) 0x50100618) = 0x01c00278; // Mask offset and count
  *((volatile uint32_t *) 0x50100698) = 0x0000000b; // TRAM ptr max
  *((volatile uint32_t *) 0x50100798) = 0x00022000; // Post processing register
  *((volatile uint32_t *) 0x50100718) = 0xffffffff; // Mask and processor enables

  // Layer 2 group 1
  *((volatile uint32_t *) 0x50500018) = 0x0000001b; // Rows
  *((volatile uint32_t *) 0x50500098) = 0x0000001b; // Columns
  *((volatile uint32_t *) 0x50500198) = 0x00000001; // Pooling rows
  *((volatile uint32_t *) 0x50500218) = 0x00000001; // Pooling columns
  *((volatile uint32_t *) 0x50500298) = 0x00000001; // Stride
  *((volatile uint32_t *) 0x50500318) = 0x00000800; // SRAM write ptr
  *((volatile uint32_t *) 0x50500418) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x50500598) = 0x00000ba0; // Layer control
  *((volatile uint32_t *) 0x50500a18) = 0x0000b800; // Layer control 2
  *((volatile uint32_t *) 0x50500618) = 0x01c00278; // Mask offset and count
  *((volatile uint32_t *) 0x50500698) = 0x0000000b; // TRAM ptr max
  *((volatile uint32_t *) 0x50500798) = 0x00022000; // Post processing register
  *((volatile uint32_t *) 0x50500718) = 0x00ff00ff; // Mask and processor enables

  // Layer 3 group 0
  *((volatile uint32_t *) 0x5010001c) = 0x0000000b; // Rows
  *((volatile uint32_t *) 0x5010009c) = 0x0000000b; // Columns
  *((volatile uint32_t *) 0x5010041c) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x5010051c) = 0x00000800; // SRAM read ptr
  *((volatile uint32_t *) 0x5010059c) = 0x00002b20; // Layer control
  *((volatile uint32_t *) 0x50100a1c) = 0x0000b800; // Layer control 2
  *((volatile uint32_t *) 0x5010061c) = 0x02800338; // Mask offset and count
  *((volatile uint32_t *) 0x5010069c) = 0x00000009; // TRAM ptr max
  *((volatile uint32_t *) 0x5010079c) = 0x00022000; // Post processing register
  *((volatile uint32_t *) 0x5010071c) = 0xffffffff; // Mask and processor enables

  // Layer 3 group 1
  *((volatile uint32_t *) 0x5050001c) = 0x0000000b; // Rows
  *((volatile uint32_t *) 0x5050009c) = 0x0000000b; // Columns
  *((volatile uint32_t *) 0x5050041c) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x5050051c) = 0x00000800; // SRAM read ptr
  *((volatile uint32_t *) 0x5050059c) = 0x00000b20; // Layer control
  *((volatile uint32_t *) 0x50500a1c) = 0x0000b800; // Layer control 2
  *((volatile uint32_t *) 0x5050061c) = 0x02800338; // Mask offset and count
  *((volatile uint32_t *) 0x5050069c) = 0x00000009; // TRAM ptr max
  *((volatile uint32_t *) 0x5050079c) = 0x00022000; // Post processing register
  *((volatile uint32_t *) 0x5050071c) = 0x00ff00ff; // Mask and processor enables

  // Layer 4 group 0
  *((volatile uint32_t *) 0x50100020) = 0x00000009; // Rows
  *((volatile uint32_t *) 0x501000a0) = 0x00000009; // Columns
  *((volatile uint32_t *) 0x501001a0) = 0x00000001; // Pooling rows
  *((volatile uint32_t *) 0x50100220) = 0x00000001; // Pooling columns
  *((volatile uint32_t *) 0x501002a0) = 0x00000001; // Stride
  *((volatile uint32_t *) 0x50100320) = 0x00000800; // SRAM write ptr
  *((volatile uint32_t *) 0x501003a0) = 0x00002000; // Write ptr time slot offs
  *((volatile uint32_t *) 0x501005a0) = 0x000009a0; // Layer control
  *((volatile uint32_t *) 0x50100120) = 0x00000103; // 1D
  *((volatile uint32_t *) 0x501007a0) = 0x03000000; // Post processing register
  *((volatile uint32_t *) 0x50100720) = 0x0000ffff; // Mask and processor enables

  // Layer 4 group 1
  *((volatile uint32_t *) 0x50500020) = 0x00000009; // Rows
  *((volatile uint32_t *) 0x505000a0) = 0x00000009; // Columns
  *((volatile uint32_t *) 0x505001a0) = 0x00000001; // Pooling rows
  *((volatile uint32_t *) 0x50500220) = 0x00000001; // Pooling columns
  *((volatile uint32_t *) 0x505002a0) = 0x00000001; // Stride
  *((volatile uint32_t *) 0x50500320) = 0x00008800; // SRAM write ptr
  *((volatile uint32_t *) 0x505003a0) = 0x00002000; // Write ptr time slot offs
  *((volatile uint32_t *) 0x505005a0) = 0x000009a0; // Layer control
  *((volatile uint32_t *) 0x50500120) = 0x00000103; // 1D
  *((volatile uint32_t *) 0x505007a0) = 0x03000000; // Post processing register
  *((volatile uint32_t *) 0x50500720) = 0x000000ff; // Mask and processor enables

  // Layer 5 group 0
  *((volatile uint32_t *) 0x501003a4) = 0x00000001; // Write ptr time slot offs
  *((volatile uint32_t *) 0x50100424) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x50100524) = 0x00000800; // SRAM read ptr
  *((volatile uint32_t *) 0x501005a4) = 0x00002920; // Layer control
  *((volatile uint32_t *) 0x50100a24) = 0x0000f808; // Layer control 2
  *((volatile uint32_t *) 0x50100624) = 0x1d403638; // Mask offset and count
  *((volatile uint32_t *) 0x50100124) = 0x00000100; // 1D
  *((volatile uint32_t *) 0x501006a4) = 0x00180031; // TRAM ptr max
  *((volatile uint32_t *) 0x501007a4) = 0x08063000; // Post processing register
  *((volatile uint32_t *) 0x50100724) = 0xffffffff; // Mask and processor enables

  // Layer 5 group 1
  *((volatile uint32_t *) 0x505003a4) = 0x00000001; // Write ptr time slot offs
  *((volatile uint32_t *) 0x50500424) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x50500524) = 0x00000800; // SRAM read ptr
  *((volatile uint32_t *) 0x505005a4) = 0x00000920; // Layer control
  *((volatile uint32_t *) 0x50500a24) = 0x0000f808; // Layer control 2
  *((volatile uint32_t *) 0x50500624) = 0x1d403638; // Mask offset and count
  *((volatile uint32_t *) 0x50500124) = 0x00000100; // 1D
  *((volatile uint32_t *) 0x505006a4) = 0x00180031; // TRAM ptr max
  *((volatile uint32_t *) 0x505007a4) = 0x08062000; // Post processing register
  *((volatile uint32_t *) 0x50500724) = 0x00ff00ff; // Mask and processor enables

  // Layer 6 group 0
  *((volatile uint32_t *) 0x50100328) = 0x00000800; // SRAM write ptr
  *((volatile uint32_t *) 0x501003a8) = 0x00000001; // Write ptr time slot offs
  *((volatile uint32_t *) 0x50100428) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x501005a8) = 0x00002920; // Layer control
  *((volatile uint32_t *) 0x50100a28) = 0x00007800; // Layer control 2
  *((volatile uint32_t *) 0x50100628) = 0x37203798; // Mask offset and count
  *((volatile uint32_t *) 0x50100128) = 0x00000100; // 1D
  *((volatile uint32_t *) 0x50100728) = 0xffffffff; // Mask and processor enables

  // Layer 6 group 1
  *((volatile uint32_t *) 0x50500328) = 0x00000800; // SRAM write ptr
  *((volatile uint32_t *) 0x505003a8) = 0x00000001; // Write ptr time slot offs
  *((volatile uint32_t *) 0x50500428) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x505005a8) = 0x00000920; // Layer control
  *((volatile uint32_t *) 0x50500a28) = 0x00007800; // Layer control 2
  *((volatile uint32_t *) 0x50500628) = 0x37203798; // Mask offset and count
  *((volatile uint32_t *) 0x50500128) = 0x00000100; // 1D
  *((volatile uint32_t *) 0x505007a8) = 0x00001000; // Post processing register
  *((volatile uint32_t *) 0x50500728) = 0xffffffff; // Mask and processor enables

  // Layer 7 group 0
  *((volatile uint32_t *) 0x501003ac) = 0x00000001; // Write ptr time slot offs
  *((volatile uint32_t *) 0x5010042c) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x5010052c) = 0x00000800; // SRAM read ptr
  *((volatile uint32_t *) 0x501005ac) = 0x00012920; // Layer control
  *((volatile uint32_t *) 0x50100a2c) = 0x00001800; // Layer control 2
  *((volatile uint32_t *) 0x5010062c) = 0x38403858; // Mask offset and count
  *((volatile uint32_t *) 0x5010012c) = 0x00000100; // 1D
  *((volatile uint32_t *) 0x501007ac) = 0x00002000; // Post processing register
  *((volatile uint32_t *) 0x5010072c) = 0xffffffff; // Mask and processor enables

  // Layer 7 group 1
  *((volatile uint32_t *) 0x505003ac) = 0x00000001; // Write ptr time slot offs
  *((volatile uint32_t *) 0x5050042c) = 0x00002000; // Write ptr mask offs
  *((volatile uint32_t *) 0x5050052c) = 0x00000800; // SRAM read ptr
  *((volatile uint32_t *) 0x505005ac) = 0x00010920; // Layer control
  *((volatile uint32_t *) 0x50500a2c) = 0x00001800; // Layer control 2
  *((volatile uint32_t *) 0x5050062c) = 0x38403858; // Mask offset and count
  *((volatile uint32_t *) 0x5050012c) = 0x00000100; // 1D
  *((volatile uint32_t *) 0x505007ac) = 0x00003010; // Post processing register


  return CNN_OK;
}

int cnn_start(void)
{
  cnn_time = 0;

  *((volatile uint32_t *) 0x50100000) = 0x00100808; // Enable group 0
  *((volatile uint32_t *) 0x50500000) = 0x00100809; // Enable group 1

#ifdef CNN_INFERENCE_TIMER
  MXC_TMR_SW_Start(CNN_INFERENCE_TIMER);
#endif

  CNN_START; // Allow capture of processing time
  *((volatile uint32_t *) 0x50100000) = 0x00100009; // Master enable group 0

  return CNN_OK;
}

// Custom unload for this network: 32-bit data, shape: [4, 1, 1]
int cnn_unload(uint32_t *out_buf)
{
  volatile uint32_t *addr;

  addr = (volatile uint32_t *) 0x50400000;
  *out_buf++ = *addr++;
  *out_buf++ = *addr++;
  *out_buf++ = *addr++;
  *out_buf++ = *addr++;

  return CNN_OK;
}

int cnn_enable(uint32_t clock_source, uint32_t clock_divider)
{
  // Reset all domains, restore power to CNN
  MXC_GCFR->reg3 = 0xf; // Reset
  MXC_GCFR->reg1 = 0x3; // Mask memory
  MXC_GCFR->reg0 = 0x3; // Power
  MXC_GCFR->reg2 = 0xc; // Iso
  MXC_GCFR->reg3 = 0x0; // Reset

  MXC_GCR->pclkdiv = (MXC_GCR->pclkdiv & ~(MXC_F_GCR_PCLKDIV_CNNCLKDIV | MXC_F_GCR_PCLKDIV_CNNCLKSEL))
                     | clock_divider | clock_source;
  MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_CNN); // Enable CNN clock

  NVIC_SetVector(CNN_IRQn, CNN_ISR); // Set CNN complete vector

  return CNN_OK;
}

int cnn_boost_enable(mxc_gpio_regs_t *port, uint32_t pin)
{
  mxc_gpio_cfg_t gpio_out;
  gpio_out.port = port;
  gpio_out.mask = pin;
  gpio_out.pad = MXC_GPIO_PAD_NONE;
  gpio_out.func = MXC_GPIO_FUNC_OUT;
  MXC_GPIO_Config(&gpio_out);
  MXC_GPIO_OutSet(gpio_out.port, gpio_out.mask);

  return CNN_OK;
}

int cnn_boost_disable(mxc_gpio_regs_t *port, uint32_t pin)
{
  mxc_gpio_cfg_t gpio_out;
  gpio_out.port = port;
  gpio_out.mask = pin;
  gpio_out.pad = MXC_GPIO_PAD_NONE;
  gpio_out.func = MXC_GPIO_FUNC_OUT;
  MXC_GPIO_Config(&gpio_out);
  MXC_GPIO_OutSet(gpio_out.port, gpio_out.mask);

  return CNN_OK;
}

int cnn_disable(void)
{
  // Disable CNN clock
  MXC_SYS_ClockDisable(MXC_SYS_PERIPH_CLOCK_CNN);

  // Disable power to CNN
  MXC_GCFR->reg3 = 0xf; // Reset
  MXC_GCFR->reg2 = 0xf; // Iso
  MXC_GCFR->reg0 = 0x0; // Power
  MXC_GCFR->reg1 = 0x0; // Mask memory
  MXC_GCFR->reg3 = 0x0; // Reset

  return CNN_OK;
}

