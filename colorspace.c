/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Colorspace conversion functions -
 *
 *  Copyright(C) 2001-2003 Peter Ross <pross@xvid.org>
 *
 *  This program is free software ; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation ; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY ; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program ; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: colorspace.c,v 1.11 2005/11/22 10:23:01 suxen_drol Exp $
 *
 ****************************************************************************/

// comment
#include <string.h>        /* memcpy */

#include "../global.h"
#include "colorspace.h"
#include "preloader.h"
#include "ple.h"
#include <arm_neon.h>
#include "neon_ans.h"

packedFuncPtr rgb555_to_yv12;
packedFuncPtr rgb565_to_yv12;
packedFuncPtr bgr_to_yv12;
packedFuncPtr bgra_to_yv12;
packedFuncPtr abgr_to_yv12;
packedFuncPtr rgba_to_yv12;
packedFuncPtr argb_to_yv12;
packedFuncPtr yuyv_to_yv12;
packedFuncPtr uyvy_to_yv12;

packedFuncPtr rgb555i_to_yv12;
packedFuncPtr rgb565i_to_yv12;
packedFuncPtr bgri_to_yv12;
packedFuncPtr bgrai_to_yv12;
packedFuncPtr abgri_to_yv12;
packedFuncPtr rgbai_to_yv12;
packedFuncPtr argbi_to_yv12;
packedFuncPtr yuyvi_to_yv12;
packedFuncPtr uyvyi_to_yv12;


packedFuncPtr yv12_to_rgb555;
packedFuncPtr yv12_to_rgb565;
packedFuncPtr yv12_to_bgr;
packedFuncPtr yv12_to_bgra;
packedFuncPtr yv12_to_abgr;
packedFuncPtr yv12_to_rgba;
packedFuncPtr yv12_to_argb;
packedFuncPtr yv12_to_yuyv;
packedFuncPtr yv12_to_uyvy;

packedFuncPtr yv12_to_rgb555i;
packedFuncPtr yv12_to_rgb565i;
packedFuncPtr yv12_to_bgri;
packedFuncPtr yv12_to_bgrai;
packedFuncPtr yv12_to_abgri;
packedFuncPtr yv12_to_rgbai;
packedFuncPtr yv12_to_argbi;
packedFuncPtr yv12_to_yuyvi;
packedFuncPtr yv12_to_uyvyi;

planarFuncPtr yv12_to_yv12;


static int32_t RGB_Y_tab[256];
static int32_t B_U_tab[256];
static int32_t G_U_tab[256];
static int32_t G_V_tab[256];
static int32_t R_V_tab[256];



void
rgb555_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
          uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
          int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
  {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
  }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t rgb, r, g, b, r0, g0, b0;
          r0 = g0 = b0 = 0;
          rgb = *(uint16_t *) (x_ptr  );
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 2) & 0xf8;
          r0 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + 2);
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 2) & 0xf8;
          r0 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + ( x_stride) );
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 2) & 0xf8;
          r0 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + ( x_stride) + 2);
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 2) & 0xf8;
          r0 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif;
      v_ptr += uv_dif;
    }
}

void rgb565_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
          uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
          int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
  {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
  }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t rgb, r, g, b, r0, g0, b0;
          r0 = g0 = b0 = 0;
          rgb = *(uint16_t *) (x_ptr  );
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 3) & 0xfc;
          r0 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + 2);
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 3) & 0xfc;
          r0 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + ( x_stride) );
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 3) & 0xfc;
          r0 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + ( x_stride) + 2);
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 3) & 0xfc;
          r0 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
   }
}
void bgr_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
           uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
           int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 3 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
  {
      x_ptr += (height - 1) * x_stride;
      x_dif = -3 * fixed_width - x_stride;
      x_stride = -x_stride;
  }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t r, g, b, r0, g0, b0;
          r0 = g0 = b0 = 0;
          r0 += r = x_ptr[2];
          g0 += g = x_ptr[1];
          b0 += b = x_ptr[0 ];
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[3 + 2];
          g0 += g = x_ptr[3 + 1];
          b0 += b = x_ptr[3 ];
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[ x_stride + 2];
          g0 += g = x_ptr[ x_stride + 1];
          b0 += b = x_ptr[ x_stride ];
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[ x_stride + 3 + 2];
          g0 += g = x_ptr[ x_stride + 3 + 1];
          b0 += b = x_ptr[ x_stride + 3 ];
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;;
          x_ptr += 2 * 3;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}
void bgra_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
  {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
  }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t r, g, b, r0, g0, b0;
          r0 = g0 = b0 = 0;
          r0 += r = x_ptr[2];
          g0 += g = x_ptr[1];
          b0 += b = x_ptr[0 ];
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[6];
          g0 += g = x_ptr[5];
          b0 += b = x_ptr[4 ];
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[ x_stride + 2];
          g0 += g = x_ptr[ x_stride + 1];
          b0 += b = x_ptr[ x_stride ];
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[ x_stride + 6];
          g0 += g = x_ptr[ x_stride + 5];
          b0 += b = x_ptr[ x_stride + 4 ];
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
   }
}
void abgr_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
  {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
  }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t r, g, b, r0, g0, b0;
          r0 = g0 = b0 = 0;
          r0 += r = x_ptr[3];
          g0 += g = x_ptr[2];
          b0 += b = x_ptr[1];
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[0 + 7];
          g0 += g = x_ptr[6];
          b0 += b = x_ptr[5];
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[ x_stride + 3];
          g0 += g = x_ptr[ x_stride + 2];
          b0 += b = x_ptr[ x_stride + 1];
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[ x_stride + 7];
          g0 += g = x_ptr[ x_stride + 6];
          b0 += b = x_ptr[ x_stride + 5];
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}
void rgba_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
  {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
  }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t r, g, b, r0, g0, b0;
          r0 = g0 = b0 = 0;
          r0 += r = x_ptr[0 ];
          g0 += g = x_ptr[1];
          b0 += b = x_ptr[2];
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[4 ];
          g0 += g = x_ptr[5];
          b0 += b = x_ptr[6];
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[ x_stride ];
          g0 += g = x_ptr[ x_stride + 1];
          b0 += b = x_ptr[ x_stride + 2];
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[ x_stride + 4 ];
          g0 += g = x_ptr[ x_stride + 5];
          b0 += b = x_ptr[ x_stride + 6];
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}
void argb_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
  {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
  }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t r, g, b, r0, g0, b0;
          r0 = g0 = b0 = 0;
          r0 += r = x_ptr[1];
          g0 += g = x_ptr[2];
          b0 += b = x_ptr[3];
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[5];
          g0 += g = x_ptr[6];
          b0 += b = x_ptr[0 + 7];
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[ x_stride + 1];
          g0 += g = x_ptr[ x_stride + 2];
          b0 += b = x_ptr[ x_stride + 3];
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[ x_stride + 5];
          g0 += g = x_ptr[ x_stride + 6];
          b0 += b = x_ptr[ x_stride + 7];
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}
void yuyv_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          y_ptr[0 ] = x_ptr[0 ];
          y_ptr[1] = x_ptr[2];
          y_ptr[ y_stride ] = x_ptr[ x_stride ];
          y_ptr[ y_stride + 1] = x_ptr[ x_stride + 2];
          u_ptr[0] =
            (x_ptr[1] + x_ptr[ x_stride + 1] +
             1) / 2;
          v_ptr[0] =
            (x_ptr[3] + x_ptr[ x_stride + 3] +
             1) / 2;;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
        }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}
void uyvy_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          y_ptr[0 ] = x_ptr[1];
          y_ptr[1] = x_ptr[3];
          y_ptr[ y_stride ] = x_ptr[ x_stride + 1];
          y_ptr[ y_stride + 1] = x_ptr[ x_stride + 3];
          u_ptr[0] =
            (x_ptr[0 ] + x_ptr[ x_stride ] +
             1) / 2;
          v_ptr[0] =
            (x_ptr[2] + x_ptr[ x_stride + 2] +
             1) / 2;;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}

void
rgb555i_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
           uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
           int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
  {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
  }
  for (y = 0; y < height; y += 4)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t rgb, r, g, b, r0, g0, b0, r1, g1, b1;
          r0 = g0 = b0 = r1 = g1 = b1 = 0;
          rgb = *(uint16_t *) (x_ptr  );
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 2) & 0xf8;
          r0 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr  + 2);
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 2) & 0xf8;
          r0 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + ( x_stride) );
          b1 += b = ((rgb) << 3) & 0xf8;
          g1 += g = ((rgb) >> 2) & 0xf8;
          r1 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + ( x_stride) + 2);
          b1 += b = ((rgb) << 3) & 0xf8;
          g1 += g = ((rgb) >> 2) & 0xf8;
          r1 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + (2 * x_stride) );
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 2) & 0xf8;
          r0 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[2 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + (2 * x_stride) + 2);
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 2) & 0xf8;
          r0 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[2 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + (3 * x_stride) );
          b1 += b = ((rgb) << 3) & 0xf8;
          g1 += g = ((rgb) >> 2) & 0xf8;
          r1 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[3 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + (3 * x_stride) + 2);
          b1 += b = ((rgb) << 3) & 0xf8;
          g1 += g = ((rgb) >> 2) & 0xf8;
          r1 += r = ((rgb) >> 7) & 0xf8;
          y_ptr[3 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          u_ptr[ uv_stride] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g1 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;
          v_ptr[ uv_stride] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g1 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void rgb565i_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
           uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
           int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t rgb, r, g, b, r0, g0, b0, r1, g1, b1;
          r0 = g0 = b0 = r1 = g1 = b1 = 0;
          rgb = *(uint16_t *) (x_ptr  );
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 3) & 0xfc;
          r0 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr  + 2);
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 3) & 0xfc;
          r0 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + ( x_stride) );
          b1 += b = ((rgb) << 3) & 0xf8;
          g1 += g = ((rgb) >> 3) & 0xfc;
          r1 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + ( x_stride) + 2);
          b1 += b = ((rgb) << 3) & 0xf8;
          g1 += g = ((rgb) >> 3) & 0xfc;
          r1 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + (2 * x_stride) );
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 3) & 0xfc;
          r0 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[2 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + (2 * x_stride) + 2);
          b0 += b = ((rgb) << 3) & 0xf8;
          g0 += g = ((rgb) >> 3) & 0xfc;
          r0 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[2 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + (3 * x_stride) );
          b1 += b = ((rgb) << 3) & 0xf8;
          g1 += g = ((rgb) >> 3) & 0xfc;
          r1 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[3 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          rgb = *(uint16_t *) (x_ptr + (3 * x_stride) + 2);
          b1 += b = ((rgb) << 3) & 0xf8;
          g1 += g = ((rgb) >> 3) & 0xfc;
          r1 += r = ((rgb) >> 8) & 0xf8;
          y_ptr[3 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          u_ptr[ uv_stride] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g1 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;
          v_ptr[ uv_stride] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g1 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void
bgri_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 3 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
  {
      x_ptr += (height - 1) * x_stride;
      x_dif = -3 * fixed_width - x_stride;
      x_stride = -x_stride;
  }
  for (y = 0; y < height; y += 4)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t r, g, b, r0, g0, b0, r1, g1, b1;
          r0 = g0 = b0 = r1 = g1 = b1 = 0;
          r0 += r = x_ptr[2];
          g0 += g = x_ptr[1];
          b0 += b = x_ptr[0 ];
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[3 + 2];
          g0 += g = x_ptr[3 + 1];
          b0 += b = x_ptr[3 ];
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[ x_stride + 2];
          g1 += g = x_ptr[ x_stride + 1];
          b1 += b = x_ptr[ x_stride ];
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[ x_stride + 3 + 2];
          g1 += g = x_ptr[ x_stride + 3 + 1];
          b1 += b = x_ptr[ x_stride + 3 ];
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[2 * x_stride + 2];
          g0 += g = x_ptr[2 * x_stride + 1];
          b0 += b = x_ptr[2 * x_stride ];
          y_ptr[2 * y_stride] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[2 * x_stride + 3 + 2];
          g0 += g = x_ptr[2 * x_stride + 3 + 1];
          b0 += b = x_ptr[2 * x_stride + 3 ];
          y_ptr[2 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[3 * x_stride + 2];
          g1 += g = x_ptr[3 * x_stride + 1];
          b1 += b = x_ptr[3 * x_stride ];
          y_ptr[3 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[3 * x_stride + 3 + 2];
          g1 += g = x_ptr[3 * x_stride + 3 + 1];
          b1 += b = x_ptr[3 * x_stride + 3 ];
          y_ptr[3 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          u_ptr[ uv_stride] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g1 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;
          v_ptr[ uv_stride] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g1 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;;
          x_ptr += 2 * 3;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void bgrai_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t r, g, b, r0, g0, b0, r1, g1, b1;
          r0 = g0 = b0 = r1 = g1 = b1 = 0;
          r0 += r = x_ptr[2];
          g0 += g = x_ptr[1];
          b0 += b = x_ptr[0 ];
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[6];
          g0 += g = x_ptr[5];
          b0 += b = x_ptr[4 ];
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[ x_stride + 2];
          g1 += g = x_ptr[ x_stride + 1];
          b1 += b = x_ptr[ x_stride ];
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[ x_stride + 6];
          g1 += g = x_ptr[ x_stride + 5];
          b1 += b = x_ptr[ x_stride + 4 ];
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[2 * x_stride + 2];
          g0 += g = x_ptr[2 * x_stride + 1];
          b0 += b = x_ptr[2 * x_stride ];
          y_ptr[2 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[2 * x_stride + 6];
          g0 += g = x_ptr[2 * x_stride + 5];
          b0 += b = x_ptr[2 * x_stride + 4 ];
          y_ptr[2 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[3 * x_stride + 2];
          g1 += g = x_ptr[3 * x_stride + 1];
          b1 += b = x_ptr[3 * x_stride ];
          y_ptr[3 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[3 * x_stride + 6];
          g1 += g = x_ptr[3 * x_stride + 5];
          b1 += b = x_ptr[3 * x_stride + 4 ];
          y_ptr[3 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          u_ptr[ uv_stride] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g1 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;
          v_ptr[ uv_stride] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g1 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void abgri_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t r, g, b, r0, g0, b0, r1, g1, b1;
          r0 = g0 = b0 = r1 = g1 = b1 = 0;
          r0 += r = x_ptr[3];
          g0 += g = x_ptr[2];
          b0 += b = x_ptr[1];
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[0 + 7];
          g0 += g = x_ptr[6];
          b0 += b = x_ptr[5];
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[ x_stride + 3];
          g1 += g = x_ptr[ x_stride + 2];
          b1 += b = x_ptr[ x_stride + 1];
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[ x_stride + 7];
          g1 += g = x_ptr[ x_stride + 6];
          b1 += b = x_ptr[ x_stride + 5];
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[2 * x_stride + 3];
          g0 += g = x_ptr[2 * x_stride + 2];
          b0 += b = x_ptr[2 * x_stride + 1];
          y_ptr[2 * y_stride] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[2 * x_stride + 7];
          g0 += g = x_ptr[2 * x_stride + 6];
          b0 += b = x_ptr[2 * x_stride + 5];
          y_ptr[2 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[3 * x_stride + 3];
          g1 += g = x_ptr[3 * x_stride + 2];
          b1 += b = x_ptr[3 * x_stride + 1];
          y_ptr[3 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[3 * x_stride + 7];
          g1 += g = x_ptr[3 * x_stride + 6];
          b1 += b = x_ptr[3 * x_stride + 5];
          y_ptr[3 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          u_ptr[ uv_stride] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g1 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;
          v_ptr[ uv_stride] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g1 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void rgbai_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t r, g, b, r0, g0, b0, r1, g1, b1;
          r0 = g0 = b0 = r1 = g1 = b1 = 0;
          r0 += r = x_ptr[0 ];
          g0 += g = x_ptr[1];
          b0 += b = x_ptr[2];
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[4 ];
          g0 += g = x_ptr[5];
          b0 += b = x_ptr[6];
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[ x_stride ];
          g1 += g = x_ptr[ x_stride + 1];
          b1 += b = x_ptr[ x_stride + 2];
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[ x_stride + 4 ];
          g1 += g = x_ptr[ x_stride + 5];
          b1 += b = x_ptr[ x_stride + 6];
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[2 * x_stride ];
          g0 += g = x_ptr[2 * x_stride + 1];
          b0 += b = x_ptr[2 * x_stride + 2];
          y_ptr[2 * y_stride] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[2 * x_stride + 4 ];
          g0 += g = x_ptr[2 * x_stride + 5];
          b0 += b = x_ptr[2 * x_stride + 6];
          y_ptr[2 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[3 * x_stride ];
          g1 += g = x_ptr[3 * x_stride + 1];
          b1 += b = x_ptr[3 * x_stride + 2];
          y_ptr[3 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[3 * x_stride + 4 ];
          g1 += g = x_ptr[3 * x_stride + 5];
          b1 += b = x_ptr[3 * x_stride + 6];
          y_ptr[3 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          u_ptr[ uv_stride] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g1 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;
          v_ptr[ uv_stride] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g1 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void argbi_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          uint32_t r, g, b, r0, g0, b0, r1, g1, b1;
          r0 = g0 = b0 = r1 = g1 = b1 = 0;
          r0 += r = x_ptr[1];
          g0 += g = x_ptr[2];
          b0 += b = x_ptr[3];
          y_ptr[0 ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[5];
          g0 += g = x_ptr[6];
          b0 += b = x_ptr[0 + 7];
          y_ptr[1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[ x_stride + 1];
          g1 += g = x_ptr[ x_stride + 2];
          b1 += b = x_ptr[ x_stride + 3];
          y_ptr[ y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[ x_stride + 5];
          g1 += g = x_ptr[ x_stride + 6];
          b1 += b = x_ptr[ x_stride + 7];
          y_ptr[ y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[2 * x_stride + 1];
          g0 += g = x_ptr[2 * x_stride + 2];
          b0 += b = x_ptr[2 * x_stride + 3];
          y_ptr[2 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r0 += r = x_ptr[2 * x_stride + 5];
          g0 += g = x_ptr[2 * x_stride + 6];
          b0 += b = x_ptr[2 * x_stride + 7];
          y_ptr[2 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[3 * x_stride + 1];
          g1 += g = x_ptr[3 * x_stride + 2];
          b1 += b = x_ptr[3 * x_stride + 3];
          y_ptr[3 * y_stride ] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          r1 += r = x_ptr[3 * x_stride + 5];
          g1 += g = x_ptr[3 * x_stride + 6];
          b1 += b = x_ptr[3 * x_stride + 7];
          y_ptr[3 * y_stride + 1] =
            (uint8_t) ((((uint16_t) ((0.257) * (1L << 8) + 0.5)) * r +
                ((uint16_t) ((0.504) * (1L << 8) + 0.5)) * g +
                ((uint16_t) ((0.098) * (1L << 8) + 0.5)) * b) >> 8) +
            16;
          u_ptr[0] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g0 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          v_ptr[0] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r0 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g0 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b0) >> (8 +
                                           2))
            + 128;
          u_ptr[ uv_stride] =
            (uint8_t) ((-((uint16_t) ((0.148) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.291) * (1L << 8) + 0.5)) * g1 +
                ((uint16_t) ((0.439) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;
          v_ptr[ uv_stride] =
            (uint8_t) ((((uint16_t) ((0.439) * (1L << 8) + 0.5)) * r1 -
                ((uint16_t) ((0.368) * (1L << 8) + 0.5)) * g1 -
                ((uint16_t) ((0.071) * (1L << 8) + 0.5)) * b1) >> (8 +
                                           2))
            + 128;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void yuyvi_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          y_ptr[0 ] = x_ptr[0 ];
          y_ptr[1] = x_ptr[2];
          y_ptr[ y_stride ] = x_ptr[ x_stride ];
          y_ptr[ y_stride + 1] = x_ptr[ x_stride + 2];
          y_ptr[2 * y_stride ] = x_ptr[2 * x_stride ];
          y_ptr[2 * y_stride + 1] = x_ptr[2 * x_stride + 2];
          y_ptr[3 * y_stride ] = x_ptr[3 * x_stride ];
          y_ptr[3 * y_stride + 1] = x_ptr[3 * x_stride + 2];
          u_ptr[0] =
            (x_ptr[1] + x_ptr[2 * x_stride + 1] +
             1) / 2;
          v_ptr[0] =
            (x_ptr[3] + x_ptr[2 * x_stride + 3] +
             1) / 2;
          u_ptr[ uv_stride] =
            (x_ptr[ x_stride + 1] + x_ptr[3 * x_stride + 1] +
             1) / 2;
          v_ptr[ uv_stride] =
            (x_ptr[ x_stride + 3] + x_ptr[3 * x_stride + 3] +
             1) / 2;;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void
uyvyi_to_yv12_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          y_ptr[0 ] = x_ptr[1];
          y_ptr[1] = x_ptr[3];
          y_ptr[ y_stride ] = x_ptr[ x_stride + 1];
          y_ptr[ y_stride + 1] = x_ptr[ x_stride + 3];
          y_ptr[2 * y_stride ] = x_ptr[2 * x_stride + 1];
          y_ptr[2 * y_stride + 1] = x_ptr[2 * x_stride + 3];
          y_ptr[3 * y_stride ] = x_ptr[3 * x_stride + 1];
          y_ptr[3 * y_stride + 1] = x_ptr[3 * x_stride + 3];
          u_ptr[0] =
            (x_ptr[0 ] + x_ptr[2 * x_stride ] +
             1) / 2;
          v_ptr[0] =
            (x_ptr[2] + x_ptr[2 * x_stride + 2] +
             1) / 2;
          u_ptr[ uv_stride] =
            (x_ptr[ x_stride ] + x_ptr[3 * x_stride ] +
             1) / 2;
          v_ptr[ uv_stride] =
            (x_ptr[ x_stride + 2] + x_ptr[3 * x_stride + 2] +
             1) / 2;;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}








         








void yv12_to_rgb555_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
          uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
          int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
  {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
  }

for(y=height; y > 0; y-=2)
{
      // ------------------ NEON ----------------------
    
      // Raw YUV
      uint8x8x2_t ystrip_top;
      uint8x8x2_t ystrip_bottom;
      uint8x8_t ustrip;
      uint8x8_t vstrip;
      
      // Intermediate YUV 32bit signed
      int16x8_t qtmp;
      int16x4_t dhigh, dlow; 
      int32x4_t rv0,rv1,guv0,guv1,bu0,bu1;
      int32x4_t rgby_top0,rgby_top1,rgby_top2,rgby_top3;
      int32x4_t rgby_bot0,rgby_bot1,rgby_bot2,rgby_bot3;
      
      // RGB values
      uint8x8_t bb,rr,gg;
      int16x4_t rr0,rr1,gg0,gg1,bb0,bb1;
      uint16x8_t rgb;
      int16x4x2_t zip;
      
      // ------------------ NEON ----------------------

      int r[2], g[2], b[2];

      r[0] = r[1] = g[0] = g[1] = b[0] = b[1] = 0;

      for (x = 0; x < fixed_width; x +=16)
      {
          // NEON //      
          
          //------------------------------------------------------------------
          // #1 Load YUV data into a vector (16 x 2 rows pixels worth of data)
          //------------------------------------------------------------------
          
          
          // PRACTICAL 10.1.2
          // ----------------
          // The following NEON variables have been declared above for use in this section:
          //  uint8x8x2_t ystrip_top;    - Next 16 elements of Y data, current row (y_ptr)
          //  uint8x8x2_t ystrip_bottom; - Next 16 elements of Y data, next row    (y_ptr + y_stride)
          //  uint8x8_t ustrip;          - Next 8 elements of U data, current row  (u_ptr)
          //  uint8x8_t vstrip;          - Next 8 elements of V data, current row  (v_ptr)
          
          // The following NEON intrinsics will be used here:
          //    vld2_u8()
          //    vld1_u8()
          // Refer to the ARM ARM and the arm_neon.h header file for more information.
          
          // TODO: Comment out "NEON_LOAD" and use the current values of y_ptr (Luminance), v_ptr
          // (Chroma red) and u_ptr (Chroma blue) to load YUV data to produce a matrix of 16x2 pixels.
                    

          //NEON_LOAD

          ystrip_top = vld2_u8(y_ptr); \
          ystrip_bottom = vld2_u8(y_ptr + y_stride); \
          ustrip = vld1_u8(u_ptr);\
          vstrip = vld1_u8(v_ptr);
          // ----------------
        

          //------------------------------------------------------------------          
          // #2 Compute RV, GV, GU, BU, Y for all pixels
          //------------------------------------------------------------------
          
                    
          // PRACTICAL 10.1.3
          // ----------------
          // The following NEON variables have been declared above for use in this section:
          //  uint8x8_t vstrip;       - contains 8 elements of V data
          //  int16x8_t qtmp;         - temporary register
          //  int16x4_t dhigh, dlow;  - temporary register
          //  int32x4_t rv0,rv1;      - destination registers containing 8 RV values (32bits each)
          
          // The following NEON intrinsics will be used here:
          //    vmovl_u8()
          //    vreinterpretq_s16_u16()
          //    vmovq_n_s16()
          //    vsubq_s16()
          //    vget_high_s16()
          //    vget_low_s16()
          //    vmull_n_s16()
          // Refer to the ARM ARM and the arm_neon.h header file for more information.
          
          // TODO:  Comment out "COMPUTE_RV".
          //        RV = 13074 × (v - 128) 
          //        Input data is 8-bits in size
          //        Results should be 32-bits signed
          

          //COMPUTE_RV

          qtmp = vreinterpretq_s16_u16( vmovl_u8(vstrip) ); \
          qtmp = vsubq_s16(qtmp, vmovq_n_s16(128)); \
          dhigh = vget_high_s16(qtmp); \
          dlow  = vget_low_s16(qtmp); \
          rv1 = vmull_n_s16(dhigh,13074); \
          rv0 = vmull_n_s16(dlow,13074);
          // ----------------
            
          // GV = 6660 × (v - 128) 

          qtmp = vreinterpretq_s16_u16( vmovl_u8(vstrip) );
          qtmp = vsubq_s16(qtmp, vmovq_n_s16(128));   
          dhigh = vget_high_s16(qtmp);
          dlow  = vget_low_s16(qtmp);
          guv1 = vmull_n_s16(dhigh,6660);
          guv0 = vmull_n_s16(dlow,6660);        
          
          // GU = 3203 × (u - 128) 

          qtmp = vreinterpretq_s16_u16( vmovl_u8(ustrip) );
          qtmp = vsubq_s16(qtmp, vmovq_n_s16(128));   
          dhigh = vget_high_s16(qtmp);
          dlow  = vget_low_s16(qtmp);
          guv1 = vaddq_s32(guv1, vmull_n_s16(dhigh,3203));
          guv0 = vaddq_s32(guv0, vmull_n_s16(dlow,3203));    
          
          // Note: GU + GV = GUV (GUV alreadu calulated above)          
          
          // BU = 16531 × (u - 128) 

          qtmp = vreinterpretq_s16_u16( vmovl_u8(ustrip) );
          qtmp = vsubq_s16(qtmp, vmovq_n_s16(128));   
          dhigh = vget_high_s16(qtmp);
          dlow  = vget_low_s16(qtmp);
          bu1 = vmull_n_s16(dhigh,16531);
          bu0 = vmull_n_s16(dlow,16531);    
          
          // Y = 9535 × (y - 16) 
           
          qtmp = vreinterpretq_s16_u16( vmovl_u8(ystrip_top.val[0]) ); 
          qtmp = vsubq_s16(qtmp, vmovq_n_s16(16));   
          dhigh = vget_high_s16(qtmp); 
          dlow  = vget_low_s16(qtmp); 
          rgby_top1 = vmull_n_s16(dhigh,9535); 
          rgby_top0 = vmull_n_s16(dlow,9535); 
        
          qtmp = vreinterpretq_s16_u16( vmovl_u8(ystrip_top.val[1]) ); 
          qtmp = vsubq_s16(qtmp, vmovq_n_s16(16)); 
          dhigh = vget_high_s16(qtmp); 
          dlow  = vget_low_s16(qtmp); 
          rgby_top3 = vmull_n_s16(dhigh,9535); 
          rgby_top2 = vmull_n_s16(dlow,9535);     
          
          qtmp = vreinterpretq_s16_u16( vmovl_u8(ystrip_bottom.val[0]) );
          qtmp = vsubq_s16(qtmp, vmovq_n_s16(16));   
          dhigh = vget_high_s16(qtmp);
          dlow  = vget_low_s16(qtmp);
          rgby_bot1 = vmull_n_s16(dhigh,9535);
          rgby_bot0 = vmull_n_s16(dlow,9535);
          
          qtmp = vreinterpretq_s16_u16( vmovl_u8(ystrip_bottom.val[1]) );
          qtmp = vsubq_s16(qtmp, vmovq_n_s16(16));   
          dhigh = vget_high_s16(qtmp);
          dlow  = vget_low_s16(qtmp);
          rgby_bot3 = vmull_n_s16(dhigh,9535);
          rgby_bot2 = vmull_n_s16(dlow,9535);    
          
          //------------------------------------------------------------------          
          // #3 Compute R, G, B and Saturate for all pixels (8 pixels)
          //------------------------------------------------------------------          
  
          //  b = ((Y + BU) >> 13);   
          //  g = ((Y - GUV) >> 13); 
          //  r = ((Y + RV) >> 13);         
          
          // First row of 0-7 pixels
          
          
          // PRACTICAL 10.1.4
          // ----------------
          // The following NEON variables have been declared above for use in this section:
          // int32x4_t rgby_top0;  - Luminance data, first 4 ODD pixels 
          // int32x4_t rgby_top2;  - Luminance data, first 4 EVEN pixels
          // int32x4_t rv0;        - first 4 RV values
          // int16x4_t rr0,rr1;    - intermediate results
          // int16x4x2_t zip;      - interleaving result
          // uint8x8_t rr;         - 8 pixels of red data
            
          // The following NEON intrinsics will be used here:
          //    vaddq_s32()
          //    vshrn_n_s32()
          //    vzip_s16()
          //    vcombine_s16()
          //    vqmovun_s16()
          // Refer to the ARM ARM and the arm_neon.h header file for more information.
           
          // TODO:  Comment out "COMPUTE_RED".
          //        r = ((Y + RV) >> 13);  
          //        Input data is signed 32-bit elements
          //        Results should be signed 16-bits elements 
          

          //COMPUTE_RED

          rr0 = vshrn_n_s32(vaddq_s32(rgby_top0,rv0),13); \
          rr1 = vshrn_n_s32(vaddq_s32(rgby_top2,rv0),13); \
          zip = vzip_s16(rr0,rr1); \
          rr   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1])); 
          // ----------------
          
          
          bb0 = vshrn_n_s32(vaddq_s32(rgby_top0,bu0),13);
          bb1 = vshrn_n_s32(vaddq_s32(rgby_top2,bu0),13);
          zip = vzip_s16(bb0,bb1);
          bb   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
                    
          gg0 = vshrn_n_s32(vsubq_s32(rgby_top0,guv0),13);
          gg1 = vshrn_n_s32(vsubq_s32(rgby_top2,guv0),13);
          zip = vzip_s16(gg0,gg1);
          gg   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
          
          
          //------------------------------------------------------------------          
          // #4 Pack and mask pixels into RGB555 format
          //------------------------------------------------------------------    
          
          //    pixel = ((rr << 7) & 0x7c00) | ((gg << 2) & 0x03e0) | ((bb >> 3) & 0x001f); 

          // Add B 
          // rgb = ((bb >> 3) & 0x001f)
          
          rgb = vmovl_u8( vand_u8(vshr_n_u8(bb,3), vmov_n_u8(0x1f)) );
          
          // Add G
          // rgb = ((gg << 2) & 0x03e0) | rgb
          rgb = vorrq_u16(rgb, vandq_u16(vshll_n_u8(gg,2), vmovq_n_u16(0x03e0)) );
          
          // PRACTICAL 10.1.5
          // ----------------
          // The following NEON variables have been declared above for use in this section:
          // uint16x8_t rgb;       - 8 pixels of 16-bits each (RGB555)
          // uint8x8_t rr;         - 8 pixels red data
              
          // The following NEON intrinsics will be used here:
          //    vshll_n_u8()
          //    vmovq_n_u16()
          //    vandq_u16()
          //    vorrq_u16()
          // Refer to the ARM ARM and the arm_neon.h header file for more information.
          
          // TODO:  Comment out "COMPUTE_RGB".
          //        rgb = ((rr << 7) & 0x7c00) | rgb
          

          //COMPUTE_RGB

          rgb = vorrq_u16(rgb, vandq_u16(vshll_n_u8(rr,7), vmovq_n_u16(0x7c00)) );
          // ----------------
          
          //------------------------------------------------------------------          
          // #5 Store RGB pixels into the framebuffer
          //------------------------------------------------------------------    
      
          // PRACTICAL 10.1.6
          // ----------------
          // The following NEON variables have been declared above for use in this section:
          // uint16x8_t rgb;       - 8 pixels of 16-bits each (RGB555)
               
          // The following NEON intrinsics will be used here:
          //    vst1q_u16()
          // Refer to the ARM ARM and the arm_neon.h header file for more information.
           
          // TODO:  Comment out "SAVE_RGB". Save rgb to the current framebuffer pointer (x_ptr)
          

          // SAVE_RGB

          vst1q_u16((uint16_t *)x_ptr,rgb);

          // ----------------
          
          //------------------------------------------------------------------          
          // #3 Compute R, G, B and Saturate for all pixels (8 pixels)
          //------------------------------------------------------------------          
  
          //  b = ((Y + BU) >> 13);   
          //  g = ((Y - GUV) >> 13); 
          //  r = ((Y + RV) >> 13);            
          
          
          // First row of 8-15 pixels
          bb0 = vshrn_n_s32(vaddq_s32(rgby_top1,bu1),13);
          bb1 = vshrn_n_s32(vaddq_s32(rgby_top3,bu1),13);
          zip = vzip_s16(bb0,bb1);
          bb   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
          
          rr0 = vshrn_n_s32(vaddq_s32(rgby_top1,rv1),13);
          rr1 = vshrn_n_s32(vaddq_s32(rgby_top3,rv1),13);
          zip = vzip_s16(rr0,rr1);
          rr   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
          
          gg0 = vshrn_n_s32(vsubq_s32(rgby_top1,guv1),13);
          gg1 = vshrn_n_s32(vsubq_s32(rgby_top3,guv1),13);
          zip = vzip_s16(gg0,gg1);
          gg   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
          
          //------------------------------------------------------------------          
          // #4 Pack and mask pixels into RGB555 format
          //------------------------------------------------------------------    
          
          //    pixel = ((rr << 7) & 0x7c00) | ((gg << 2) & 0x03e0) | ((bb >> 3) & 0x001f); 
          
          // Add B
          rgb = vmovl_u8( vand_u8(vshr_n_u8(bb,3), vmov_n_u8(0x1f)) );
          // Add G
          rgb = vorrq_u16(rgb, vandq_u16(vshll_n_u8(gg,2), vmovq_n_u16(0x03e0)) );
          // Add R
          rgb = vorrq_u16(rgb, vandq_u16(vshll_n_u8(rr,7), vmovq_n_u16(0x7c00)) );
          
          //------------------------------------------------------------------          
          // #5 Store RGB pixels into the framebuffer
          //------------------------------------------------------------------    
      
          // Save to framebuffer
          
          vst1q_u16((uint16_t *)(x_ptr + 16),rgb);          

          //------------------------------------------------------------------          
          // #3 Compute R, G, B and Saturate for all pixels (8 pixels)
          //------------------------------------------------------------------          
  
          //  b = ((Y + BU) >> 13);   
          //  g = ((Y - GUV) >> 13); 
          //  r = ((Y + RV) >> 13);            
          
          // Second row of 0-7 pixels
          bb0 = vshrn_n_s32(vaddq_s32(rgby_bot0,bu0),13);
          bb1 = vshrn_n_s32(vaddq_s32(rgby_bot2,bu0),13);
          zip = vzip_s16(bb0,bb1);
          bb   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
          
          rr0 = vshrn_n_s32(vaddq_s32(rgby_bot0,rv0),13);
          rr1 = vshrn_n_s32(vaddq_s32(rgby_bot2,rv0),13);
          zip = vzip_s16(rr0,rr1);
          rr   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
          
          gg0 = vshrn_n_s32(vsubq_s32(rgby_bot0,guv0),13);
          gg1 = vshrn_n_s32(vsubq_s32(rgby_bot2,guv0),13);
          zip = vzip_s16(gg0,gg1);
          gg   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
          
          //------------------------------------------------------------------          
          // #4 Pack and mask pixels into RGB555 format
          //------------------------------------------------------------------    
          
          //    pixel = ((rr << 7) & 0x7c00) | ((gg << 2) & 0x03e0) | ((bb >> 3) & 0x001f); 
          
          // Add B
          rgb = vmovl_u8( vand_u8(vshr_n_u8(bb,3), vmov_n_u8(0x1f)) );
          // Add G
          rgb = vorrq_u16(rgb, vandq_u16(vshll_n_u8(gg,2), vmovq_n_u16(0x03e0)) );
          // Add R
          rgb = vorrq_u16(rgb, vandq_u16(vshll_n_u8(rr,7), vmovq_n_u16(0x7c00)) );
          
          //------------------------------------------------------------------          
          // #5 Store RGB pixels into the framebuffer
          //------------------------------------------------------------------    
      
          // Save to framebuffer
          
          vst1q_u16((uint16_t *)(x_ptr + x_stride),rgb);

          //------------------------------------------------------------------          
          // #3 Compute R, G, B and Saturate for all pixels (8 pixels)
          //------------------------------------------------------------------          
  
          //  b = ((Y + BU) >> 13);   
          //  g = ((Y - GUV) >> 13); 
          //  r = ((Y + RV) >> 13);            
          
          // Second row of 8-15 pixels
          bb0 = vshrn_n_s32(vaddq_s32(rgby_bot1,bu1),13);
          bb1 = vshrn_n_s32(vaddq_s32(rgby_bot3,bu1),13);
          zip = vzip_s16(bb0,bb1);
          bb   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
          
          rr0 = vshrn_n_s32(vaddq_s32(rgby_bot1,rv1),13);
          rr1 = vshrn_n_s32(vaddq_s32(rgby_bot3,rv1),13);
          zip = vzip_s16(rr0,rr1);
          rr   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
          
          gg0 = vshrn_n_s32(vsubq_s32(rgby_bot1,guv1),13);
          gg1 = vshrn_n_s32(vsubq_s32(rgby_bot3,guv1),13);
          zip = vzip_s16(gg0,gg1);
          gg   = vqmovun_s16( vcombine_s16 (zip.val[0],zip.val[1]));
          
          //------------------------------------------------------------------          
          // #4 Pack and mask pixels into RGB555 format
          //------------------------------------------------------------------    
          
          //    pixel = ((rr << 7) & 0x7c00) | ((gg << 2) & 0x03e0) | ((bb >> 3) & 0x001f); 
          
          // Add B
          rgb = vmovl_u8( vand_u8(vshr_n_u8(bb,3), vmov_n_u8(0x1f)) );
          // Add G
          rgb = vorrq_u16(rgb, vandq_u16(vshll_n_u8(gg,2), vmovq_n_u16(0x03e0)) );
          // Add R
          rgb = vorrq_u16(rgb, vandq_u16(vshll_n_u8(rr,7), vmovq_n_u16(0x7c00)) );
          
          //------------------------------------------------------------------          
          // #5 Store RGB pixels into the framebuffer
          //------------------------------------------------------------------    
      
          // Save to framebuffer
          
          vst1q_u16((uint16_t *)(x_ptr + 16 + x_stride ),rgb);              
          
          x_ptr += 32; // 16 pixels 
          y_ptr += 16;   // 16 Pixels
          u_ptr += 8;  // 8 Pixels (effective x2)
          v_ptr += 8;  // 8 Pixels (effective x2)
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif ;
      v_ptr += uv_dif ;
   }
}
      
void yv12_to_rgb565_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
          uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
          int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
for (y = 0; y < height; y += 2)
 
  {
      int r[2], g[2], b[2];
      r[0] = r[1] = g[0] = g[1] = b[0] = b[1] = 0;;
      for (x = 0; x < fixed_width; x += 2)
      {
          int rgb_y;
          int b_u0 = B_U_tab[u_ptr[0]];
          int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
          int r_v0 = R_V_tab[v_ptr[0]];
          rgb_y = RGB_Y_tab[y_ptr[0]];
          b[0] = (b[0] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[0] = (g[0] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[0] = (r[0] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr  ) =
            (((0 >
               (((255) < (r[0]) ? (255) : (r[0]))) ? 0 : (((255) <
                                     (r[0]) ? (255)
                                     : (r[0])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[0]) ? (255) : (g[0]))) ? 0 : (((255) <
                                     (g[0]) ? (255)
                                     : (g[0])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[0]) ? (255) : (b[0]))) ? 0 : (((255) <
                                     (b[0]) ? (255)
                                     : (b[0])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[1]];
          b[0] = (b[0] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[0] = (g[0] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[0] = (r[0] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr  + 2) =
            (((0 >
               (((255) < (r[0]) ? (255) : (r[0]))) ? 0 : (((255) <
                                     (r[0]) ? (255)
                                     : (r[0])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[0]) ? (255) : (g[0]))) ? 0 : (((255) <
                                     (g[0]) ? (255)
                                     : (g[0])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[0]) ? (255) : (b[0]))) ? 0 : (((255) <
                                     (b[0]) ? (255)
                                     : (b[0])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride  ]];
          b[1] = (b[1] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[1] = (g[1] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[1] = (r[1] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr + ( x_stride) ) =
            (((0 >
               (((255) < (r[1]) ? (255) : (r[1]))) ? 0 : (((255) <
                                     (r[1]) ? (255)
                                     : (r[1])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[1]) ? (255) : (g[1]))) ? 0 : (((255) <
                                     (g[1]) ? (255)
                                     : (g[1])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[1]) ? (255) : (b[1]))) ? 0 : (((255) <
                                     (b[1]) ? (255)
                                     : (b[1])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride  + 1]];
          b[1] = (b[1] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[1] = (g[1] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[1] = (r[1] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr + ( x_stride) + 2) =
            (((0 >
               (((255) < (r[1]) ? (255) : (r[1]))) ? 0 : (((255) <
                                     (r[1]) ? (255)
                                     : (r[1])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[1]) ? (255) : (g[1]))) ? 0 : (((255) <
                                     (g[1]) ? (255)
                                     : (g[1])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[1]) ? (255) : (b[1]))) ? 0 : (((255) <
                                     (b[1]) ? (255)
                                     : (b[1])))) >> 3)
             & 0x001f);;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      } 
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
   }
}

void yv12_to_bgr_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
           uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
           int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 3 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -3 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          int rgb_y;
          int b_u0 = B_U_tab[u_ptr[0]];
          int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
          int r_v0 = R_V_tab[v_ptr[0]];
          rgb_y = RGB_Y_tab[y_ptr[0 ]];
          x_ptr[0 ] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[1] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[2] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (3 > 3)
            x_ptr[0 ] = 0;
          rgb_y = RGB_Y_tab[y_ptr[1]];
          x_ptr[3 ] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[3 + 1] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[3 + 2] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (3 > 3)
            x_ptr[3 ] = 0;
          rgb_y = RGB_Y_tab[y_ptr[ y_stride ]];
          x_ptr[ x_stride ] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[ x_stride + 1] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[ x_stride + 2] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (3 > 3)
            x_ptr[ x_stride ] = 0;
          rgb_y = RGB_Y_tab[y_ptr[ y_stride + 1]];
          x_ptr[ x_stride + 3 ] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[ x_stride + 3 + 1] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[ x_stride + 3 + 2] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (3 > 3)
            x_ptr[ x_stride + 3 ] = 0;;
          x_ptr += 2 * 3;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}

void yv12_to_bgra_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          int rgb_y;
          int b_u0 = B_U_tab[u_ptr[0]];
          int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
          int r_v0 = R_V_tab[v_ptr[0]];
          rgb_y = RGB_Y_tab[y_ptr[0 ]];
          x_ptr[0 ] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[1] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[2] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[3] = 0;
          rgb_y = RGB_Y_tab[y_ptr[1]];
          x_ptr[4 ] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[5] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[6] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[0 + 7] = 0;
          rgb_y = RGB_Y_tab[y_ptr[ y_stride ]];
          x_ptr[ x_stride ] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[ x_stride + 1] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[ x_stride + 2] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[ x_stride + 3] = 0;
          rgb_y = RGB_Y_tab[y_ptr[ y_stride + 1]];
          x_ptr[ x_stride + 4 ] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[ x_stride + 5] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[ x_stride + 6] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[ x_stride + 7] = 0;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}
void yv12_to_abgr_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          int rgb_y;
          int b_u0 = B_U_tab[u_ptr[0]];
          int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
          int r_v0 = R_V_tab[v_ptr[0]];
          rgb_y = RGB_Y_tab[y_ptr[0 ]];
          x_ptr[1] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[2] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[3] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[0 ] = 0;
          rgb_y = RGB_Y_tab[y_ptr[1]];
          x_ptr[5] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[6] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[0 + 7] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[4 ] = 0;
          rgb_y = RGB_Y_tab[y_ptr[ y_stride ]];
          x_ptr[ x_stride + 1] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[ x_stride + 2] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[ x_stride + 3] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[ x_stride ] = 0;
          rgb_y = RGB_Y_tab[y_ptr[ y_stride + 1]];
          x_ptr[ x_stride + 5] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[ x_stride + 6] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[ x_stride + 7] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[ x_stride + 4 ] = 0;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}
void
yv12_to_rgba_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          int rgb_y;
          int b_u0 = B_U_tab[u_ptr[0]];
          int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
          int r_v0 = R_V_tab[v_ptr[0]];
          rgb_y = RGB_Y_tab[y_ptr[0 ]];
          x_ptr[2] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[1] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[0 ] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[3] = 0;
          rgb_y = RGB_Y_tab[y_ptr[1]];
          x_ptr[6] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[5] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[4 ] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[0 + 7] = 0;
          rgb_y = RGB_Y_tab[y_ptr[ y_stride ]];
          x_ptr[ x_stride + 2] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[ x_stride + 1] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[ x_stride ] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[ x_stride + 3] = 0;
          rgb_y = RGB_Y_tab[y_ptr[ y_stride + 1]];
          x_ptr[ x_stride + 6] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[ x_stride + 5] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[ x_stride + 4 ] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[ x_stride + 7] = 0;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}
void yv12_to_argb_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          int rgb_y;
          int b_u0 = B_U_tab[u_ptr[0]];
          int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
          int r_v0 = R_V_tab[v_ptr[0]];
          rgb_y = RGB_Y_tab[y_ptr[0 ]];
          x_ptr[3] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[2] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[1] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[0 ] = 0;
          rgb_y = RGB_Y_tab[y_ptr[1]];
          x_ptr[0 + 7] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[6] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[5] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[4 ] = 0;
          rgb_y = RGB_Y_tab[y_ptr[ y_stride ]];
          x_ptr[ x_stride + 3] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[ x_stride + 2] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[ x_stride + 1] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[ x_stride ] = 0;
          rgb_y = RGB_Y_tab[y_ptr[ y_stride + 1]];
          x_ptr[ x_stride + 7] =
            (0 >
             (((255) <
               ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                              b_u0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
          x_ptr[ x_stride + 6] =
            (0 >
             (((255) <
               ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                               g_uv0) >> 13))) ? 0
             : (((255) <
             ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
          x_ptr[ x_stride + 5] =
            (0 >
             (((255) <
               ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                              r_v0) >> 13))) ? 0
             : (((255) <
             ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
          if (4 > 3)
            x_ptr[ x_stride + 4 ] = 0;;
          x_ptr += 8;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}
void yv12_to_yuyv_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          x_ptr[0 ] = y_ptr[0 ];
          x_ptr[1] = u_ptr[0 ];
          x_ptr[2] = y_ptr[1];
          x_ptr[3] = v_ptr[0 ];
          x_ptr[ x_stride ] = y_ptr[ y_stride ];
          x_ptr[ x_stride + 1] = u_ptr[0 ];
          x_ptr[ x_stride + 2] = y_ptr[ y_stride + 1];
          x_ptr[ x_stride + 3] = v_ptr[0 ];;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}
void yv12_to_uyvy_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 2)
  {
      for (x = 0; x < fixed_width; x += 2)
      {
          x_ptr[1] = y_ptr[0 ];
          x_ptr[0 ] = u_ptr[0 ];
          x_ptr[3] = y_ptr[1];
          x_ptr[2] = v_ptr[0 ];
          x_ptr[ x_stride + 1] = y_ptr[ y_stride ];
          x_ptr[ x_stride ] = u_ptr[0 ];
          x_ptr[ x_stride + 3] = y_ptr[ y_stride + 1];
          x_ptr[ x_stride + 2] = v_ptr[0 ];;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif +  x_stride;
      y_ptr += y_dif +  y_stride;
      u_ptr += uv_dif + (1 - 1) * uv_stride;
      v_ptr += uv_dif + (1 - 1) * uv_stride;
    }
}

void
yv12_to_rgb555i_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
           uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
           int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
  {
      int r[4], g[4], b[4];
      r[0] = r[1] = r[2] = r[3] = 0;
      g[0] = g[1] = g[2] = g[3] = 0;
      b[0] = b[1] = b[2] = b[3] = 0;;
      for (x = 0; x < fixed_width; x += 2)
      {
          int rgb_y;
          int b_u0 = B_U_tab[u_ptr[0]];
          int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
          int r_v0 = R_V_tab[v_ptr[0]];
          int b_u1 = B_U_tab[u_ptr[uv_stride]];
          int g_uv1 = G_U_tab[u_ptr[uv_stride]] + G_V_tab[v_ptr[uv_stride]];
          int r_v1 = R_V_tab[v_ptr[uv_stride]];
          rgb_y = RGB_Y_tab[y_ptr[0]];
          b[0] = (b[0] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[0] = (g[0] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[0] = (r[0] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr  ) =
            (((0 >
               (((255) < (r[0]) ? (255) : (r[0]))) ? 0 : (((255) <
                                     (r[0]) ? (255)
                                     : (r[0])))) << 7)
             & 0x7c00) |
            (((0 >
               (((255) < (g[0]) ? (255) : (g[0]))) ? 0 : (((255) <
                                     (g[0]) ? (255)
                                     : (g[0])))) << 2)
             & 0x03e0) |
            (((0 >
               (((255) < (b[0]) ? (255) : (b[0]))) ? 0 : (((255) <
                                     (b[0]) ? (255)
                                     : (b[0])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[1]];
          b[0] = (b[0] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[0] = (g[0] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[0] = (r[0] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr  + 2) =
            (((0 >
               (((255) < (r[0]) ? (255) : (r[0]))) ? 0 : (((255) <
                                     (r[0]) ? (255)
                                     : (r[0])))) << 7)
             & 0x7c00) |
            (((0 >
               (((255) < (g[0]) ? (255) : (g[0]))) ? 0 : (((255) <
                                     (g[0]) ? (255)
                                     : (g[0])))) << 2)
             & 0x03e0) |
            (((0 >
               (((255) < (b[0]) ? (255) : (b[0]))) ? 0 : (((255) <
                                     (b[0]) ? (255)
                                     : (b[0])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride  ]];
          b[1] = (b[1] & 0x7) + ((rgb_y + b_u1) >> 13);
          g[1] = (g[1] & 0x7) + ((rgb_y - g_uv1) >> 13);
          r[1] = (r[1] & 0x7) + ((rgb_y + r_v1) >> 13);
          *(uint16_t *) (x_ptr + ( x_stride) ) =
            (((0 >
               (((255) < (r[1]) ? (255) : (r[1]))) ? 0 : (((255) <
                                     (r[1]) ? (255)
                                     : (r[1])))) << 7)
             & 0x7c00) |
            (((0 >
               (((255) < (g[1]) ? (255) : (g[1]))) ? 0 : (((255) <
                                     (g[1]) ? (255)
                                     : (g[1])))) << 2)
             & 0x03e0) |
            (((0 >
               (((255) < (b[1]) ? (255) : (b[1]))) ? 0 : (((255) <
                                     (b[1]) ? (255)
                                     : (b[1])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride  + 1]];
          b[1] = (b[1] & 0x7) + ((rgb_y + b_u1) >> 13);
          g[1] = (g[1] & 0x7) + ((rgb_y - g_uv1) >> 13);
          r[1] = (r[1] & 0x7) + ((rgb_y + r_v1) >> 13);
          *(uint16_t *) (x_ptr + ( x_stride) + 2) =
            (((0 >
               (((255) < (r[1]) ? (255) : (r[1]))) ? 0 : (((255) <
                                     (r[1]) ? (255)
                                     : (r[1])))) << 7)
             & 0x7c00) |
            (((0 >
               (((255) < (g[1]) ? (255) : (g[1]))) ? 0 : (((255) <
                                     (g[1]) ? (255)
                                     : (g[1])))) << 2)
             & 0x03e0) |
            (((0 >
               (((255) < (b[1]) ? (255) : (b[1]))) ? 0 : (((255) <
                                     (b[1]) ? (255)
                                     : (b[1])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride * 2 ]];
          b[2] = (b[2] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[2] = (g[2] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[2] = (r[2] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr + (2 * x_stride) ) =
            (((0 >
               (((255) < (r[2]) ? (255) : (r[2]))) ? 0 : (((255) <
                                     (r[2]) ? (255)
                                     : (r[2])))) << 7)
             & 0x7c00) |
            (((0 >
               (((255) < (g[2]) ? (255) : (g[2]))) ? 0 : (((255) <
                                     (g[2]) ? (255)
                                     : (g[2])))) << 2)
             & 0x03e0) |
            (((0 >
               (((255) < (b[2]) ? (255) : (b[2]))) ? 0 : (((255) <
                                     (b[2]) ? (255)
                                     : (b[2])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride * 2 + 1]];
          b[2] = (b[2] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[2] = (g[2] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[2] = (r[2] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr + (2 * x_stride) + 2) =
            (((0 >
               (((255) < (r[2]) ? (255) : (r[2]))) ? 0 : (((255) <
                                     (r[2]) ? (255)
                                     : (r[2])))) << 7)
             & 0x7c00) |
            (((0 >
               (((255) < (g[2]) ? (255) : (g[2]))) ? 0 : (((255) <
                                     (g[2]) ? (255)
                                     : (g[2])))) << 2)
             & 0x03e0) |
            (((0 >
               (((255) < (b[2]) ? (255) : (b[2]))) ? 0 : (((255) <
                                     (b[2]) ? (255)
                                     : (b[2])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride * 3 ]];
          b[3] = (b[3] & 0x7) + ((rgb_y + b_u1) >> 13);
          g[3] = (g[3] & 0x7) + ((rgb_y - g_uv1) >> 13);
          r[3] = (r[3] & 0x7) + ((rgb_y + r_v1) >> 13);
          *(uint16_t *) (x_ptr + (3 * x_stride) ) =
            (((0 >
               (((255) < (r[3]) ? (255) : (r[3]))) ? 0 : (((255) <
                                     (r[3]) ? (255)
                                     : (r[3])))) << 7)
             & 0x7c00) |
            (((0 >
               (((255) < (g[3]) ? (255) : (g[3]))) ? 0 : (((255) <
                                     (g[3]) ? (255)
                                     : (g[3])))) << 2)
             & 0x03e0) |
            (((0 >
               (((255) < (b[3]) ? (255) : (b[3]))) ? 0 : (((255) <
                                     (b[3]) ? (255)
                                     : (b[3])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride * 3 + 1]];
          b[3] = (b[3] & 0x7) + ((rgb_y + b_u1) >> 13);
          g[3] = (g[3] & 0x7) + ((rgb_y - g_uv1) >> 13);
          r[3] = (r[3] & 0x7) + ((rgb_y + r_v1) >> 13);
          *(uint16_t *) (x_ptr + (3 * x_stride) + 2) =
            (((0 >
               (((255) < (r[3]) ? (255) : (r[3]))) ? 0 : (((255) <
                                     (r[3]) ? (255)
                                     : (r[3])))) << 7)
             & 0x7c00) |
            (((0 >
               (((255) < (g[3]) ? (255) : (g[3]))) ? 0 : (((255) <
                                     (g[3]) ? (255)
                                     : (g[3])))) << 2)
             & 0x03e0) |
            (((0 >
               (((255) < (b[3]) ? (255) : (b[3]))) ? 0 : (((255) <
                                     (b[3]) ? (255)
                                     : (b[3])))) >> 3)
             & 0x001f);;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
  }
}
void
yv12_to_rgb565i_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
           uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
           int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
  {
      int r[4], g[4], b[4];
      r[0] = r[1] = r[2] = r[3] = 0;
      g[0] = g[1] = g[2] = g[3] = 0;
      b[0] = b[1] = b[2] = b[3] = 0;;
      for (x = 0; x < fixed_width; x += 2)
        {
          int rgb_y;
          int b_u0 = B_U_tab[u_ptr[0]];
          int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
          int r_v0 = R_V_tab[v_ptr[0]];
          int b_u1 = B_U_tab[u_ptr[uv_stride]];
          int g_uv1 = G_U_tab[u_ptr[uv_stride]] + G_V_tab[v_ptr[uv_stride]];
          int r_v1 = R_V_tab[v_ptr[uv_stride]];
          rgb_y = RGB_Y_tab[y_ptr[0]];
          b[0] = (b[0] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[0] = (g[0] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[0] = (r[0] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr  ) =
            (((0 >
               (((255) < (r[0]) ? (255) : (r[0]))) ? 0 : (((255) <
                                     (r[0]) ? (255)
                                     : (r[0])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[0]) ? (255) : (g[0]))) ? 0 : (((255) <
                                     (g[0]) ? (255)
                                     : (g[0])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[0]) ? (255) : (b[0]))) ? 0 : (((255) <
                                     (b[0]) ? (255)
                                     : (b[0])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[1]];
          b[0] = (b[0] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[0] = (g[0] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[0] = (r[0] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr  + 2) =
            (((0 >
               (((255) < (r[0]) ? (255) : (r[0]))) ? 0 : (((255) <
                                     (r[0]) ? (255)
                                     : (r[0])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[0]) ? (255) : (g[0]))) ? 0 : (((255) <
                                     (g[0]) ? (255)
                                     : (g[0])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[0]) ? (255) : (b[0]))) ? 0 : (((255) <
                                     (b[0]) ? (255)
                                     : (b[0])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride  ]];
          b[1] = (b[1] & 0x7) + ((rgb_y + b_u1) >> 13);
          g[1] = (g[1] & 0x7) + ((rgb_y - g_uv1) >> 13);
          r[1] = (r[1] & 0x7) + ((rgb_y + r_v1) >> 13);
          *(uint16_t *) (x_ptr + ( x_stride) ) =
            (((0 >
               (((255) < (r[1]) ? (255) : (r[1]))) ? 0 : (((255) <
                                     (r[1]) ? (255)
                                     : (r[1])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[1]) ? (255) : (g[1]))) ? 0 : (((255) <
                                     (g[1]) ? (255)
                                     : (g[1])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[1]) ? (255) : (b[1]))) ? 0 : (((255) <
                                     (b[1]) ? (255)
                                     : (b[1])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride  + 1]];
          b[1] = (b[1] & 0x7) + ((rgb_y + b_u1) >> 13);
          g[1] = (g[1] & 0x7) + ((rgb_y - g_uv1) >> 13);
          r[1] = (r[1] & 0x7) + ((rgb_y + r_v1) >> 13);
          *(uint16_t *) (x_ptr + ( x_stride) + 2) =
            (((0 >
               (((255) < (r[1]) ? (255) : (r[1]))) ? 0 : (((255) <
                                     (r[1]) ? (255)
                                     : (r[1])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[1]) ? (255) : (g[1]))) ? 0 : (((255) <
                                     (g[1]) ? (255)
                                     : (g[1])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[1]) ? (255) : (b[1]))) ? 0 : (((255) <
                                     (b[1]) ? (255)
                                     : (b[1])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride * 2 ]];
          b[2] = (b[2] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[2] = (g[2] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[2] = (r[2] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr + (2 * x_stride) ) =
            (((0 >
               (((255) < (r[2]) ? (255) : (r[2]))) ? 0 : (((255) <
                                     (r[2]) ? (255)
                                     : (r[2])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[2]) ? (255) : (g[2]))) ? 0 : (((255) <
                                     (g[2]) ? (255)
                                     : (g[2])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[2]) ? (255) : (b[2]))) ? 0 : (((255) <
                                     (b[2]) ? (255)
                                     : (b[2])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride * 2 + 1]];
          b[2] = (b[2] & 0x7) + ((rgb_y + b_u0) >> 13);
          g[2] = (g[2] & 0x7) + ((rgb_y - g_uv0) >> 13);
          r[2] = (r[2] & 0x7) + ((rgb_y + r_v0) >> 13);
          *(uint16_t *) (x_ptr + (2 * x_stride) + 2) =
            (((0 >
               (((255) < (r[2]) ? (255) : (r[2]))) ? 0 : (((255) <
                                     (r[2]) ? (255)
                                     : (r[2])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[2]) ? (255) : (g[2]))) ? 0 : (((255) <
                                     (g[2]) ? (255)
                                     : (g[2])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[2]) ? (255) : (b[2]))) ? 0 : (((255) <
                                     (b[2]) ? (255)
                                     : (b[2])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride * 3 ]];
          b[3] = (b[3] & 0x7) + ((rgb_y + b_u1) >> 13);
          g[3] = (g[3] & 0x7) + ((rgb_y - g_uv1) >> 13);
          r[3] = (r[3] & 0x7) + ((rgb_y + r_v1) >> 13);
          *(uint16_t *) (x_ptr + (3 * x_stride) ) =
            (((0 >
               (((255) < (r[3]) ? (255) : (r[3]))) ? 0 : (((255) <
                                     (r[3]) ? (255)
                                     : (r[3])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[3]) ? (255) : (g[3]))) ? 0 : (((255) <
                                     (g[3]) ? (255)
                                     : (g[3])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[3]) ? (255) : (b[3]))) ? 0 : (((255) <
                                     (b[3]) ? (255)
                                     : (b[3])))) >> 3)
             & 0x001f);
          rgb_y = RGB_Y_tab[y_ptr[y_stride * 3 + 1]];
          b[3] = (b[3] & 0x7) + ((rgb_y + b_u1) >> 13);
          g[3] = (g[3] & 0x7) + ((rgb_y - g_uv1) >> 13);
          r[3] = (r[3] & 0x7) + ((rgb_y + r_v1) >> 13);
          *(uint16_t *) (x_ptr + (3 * x_stride) + 2) =
            (((0 >
               (((255) < (r[3]) ? (255) : (r[3]))) ? 0 : (((255) <
                                     (r[3]) ? (255)
                                     : (r[3])))) << 8)
             & 0xf800) |
            (((0 >
               (((255) < (g[3]) ? (255) : (g[3]))) ? 0 : (((255) <
                                     (g[3]) ? (255)
                                     : (g[3])))) << 3)
             & 0x07e0) |
            (((0 >
               (((255) < (b[3]) ? (255) : (b[3]))) ? 0 : (((255) <
                                     (b[3]) ? (255)
                                     : (b[3])))) >> 3)
             & 0x001f);;
          x_ptr += 4;
          y_ptr += 2;
          u_ptr += 1;
          v_ptr += 1;
      } 
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
  }
}
void
yv12_to_bgri_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
        uint8_t * u_ptr, uint8_t * v_ptr, int y_stride, int uv_stride,
        int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 3 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -3 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
  {
      for (x = 0; x < fixed_width; x += 2)
    {
      int rgb_y;
      int b_u0 = B_U_tab[u_ptr[0]];
      int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
      int r_v0 = R_V_tab[v_ptr[0]];
      int b_u1 = B_U_tab[u_ptr[uv_stride]];
      int g_uv1 = G_U_tab[u_ptr[uv_stride]] + G_V_tab[v_ptr[uv_stride]];
      int r_v1 = R_V_tab[v_ptr[uv_stride]];
      rgb_y = RGB_Y_tab[y_ptr[0 ]];
      x_ptr[0 ] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (3 > 3)
        x_ptr[0 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[1]];
      x_ptr[3 ] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[3 + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[3 + 2] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (3 > 3)
        x_ptr[3 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[ y_stride ]];
      x_ptr[ x_stride ] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[ x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[ x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (3 > 3)
        x_ptr[ x_stride ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[ y_stride + 1]];
      x_ptr[ x_stride + 3 ] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[ x_stride + 3 + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[ x_stride + 3 + 2] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (3 > 3)
        x_ptr[ x_stride + 3 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[2 * y_stride ]];
      x_ptr[2 * x_stride ] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2 * x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2 * x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (3 > 3)
        x_ptr[2 * x_stride ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[2 * y_stride + 1]];
      x_ptr[2 * x_stride + 3 ] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2 * x_stride + 3 + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2 * x_stride + 3 + 2] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (3 > 3)
        x_ptr[2 * x_stride + 3 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[3 * y_stride ]];
      x_ptr[3 * x_stride ] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[3 * x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[3 * x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (3 > 3)
        x_ptr[3 * x_stride ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[3 * y_stride + 1]];
      x_ptr[3 * x_stride + 3 ] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[3 * x_stride + 3 + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[3 * x_stride + 3 + 2] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (3 > 3)
        x_ptr[3 * x_stride + 3 ] = 0;;
      x_ptr += 2 * 3;
      y_ptr += 2;
      u_ptr += 1;
      v_ptr += 1;
    }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void
yv12_to_bgrai_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
    {;
      for (x = 0; x < fixed_width; x += 2)
    {
      int rgb_y;
      int b_u0 = B_U_tab[u_ptr[0]];
      int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
      int r_v0 = R_V_tab[v_ptr[0]];
      int b_u1 = B_U_tab[u_ptr[uv_stride]];
      int g_uv1 = G_U_tab[u_ptr[uv_stride]] + G_V_tab[v_ptr[uv_stride]];
      int r_v1 = R_V_tab[v_ptr[uv_stride]];
      rgb_y = RGB_Y_tab[y_ptr[0 ]];
      x_ptr[0 ] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[3] = 0;
      rgb_y = RGB_Y_tab[y_ptr[1]];
      x_ptr[4 ] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[5] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[6] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[0 + 7] = 0;
      rgb_y = RGB_Y_tab[y_ptr[ y_stride ]];
      x_ptr[ x_stride ] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[ x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[ x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[ x_stride + 3] = 0;
      rgb_y = RGB_Y_tab[y_ptr[ y_stride + 1]];
      x_ptr[ x_stride + 4 ] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[ x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[ x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[ x_stride + 7] = 0;
      rgb_y = RGB_Y_tab[y_ptr[2 * y_stride ]];
      x_ptr[2 * x_stride ] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2 * x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2 * x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[2 * x_stride + 3] = 0;
      rgb_y = RGB_Y_tab[y_ptr[2 * y_stride + 1]];
      x_ptr[2 * x_stride + 4 ] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2 * x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2 * x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[2 * x_stride + 7] = 0;
      rgb_y = RGB_Y_tab[y_ptr[3 * y_stride ]];
      x_ptr[3 * x_stride ] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[3 * x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[3 * x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[3 * x_stride + 3] = 0;
      rgb_y = RGB_Y_tab[y_ptr[3 * y_stride + 1]];
      x_ptr[3 * x_stride + 4 ] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[3 * x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[3 * x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[3 * x_stride + 7] = 0;;
      x_ptr += 8;
      y_ptr += 2;
      u_ptr += 1;
      v_ptr += 1;
    }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void
yv12_to_abgri_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
    {;
      for (x = 0; x < fixed_width; x += 2)
    {
      int rgb_y;
      int b_u0 = B_U_tab[u_ptr[0]];
      int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
      int r_v0 = R_V_tab[v_ptr[0]];
      int b_u1 = B_U_tab[u_ptr[uv_stride]];
      int g_uv1 = G_U_tab[u_ptr[uv_stride]] + G_V_tab[v_ptr[uv_stride]];
      int r_v1 = R_V_tab[v_ptr[uv_stride]];
      rgb_y = RGB_Y_tab[y_ptr[0 ]];
      x_ptr[1] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[3] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[0 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[1]];
      x_ptr[5] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[6] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[0 + 7] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[4 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[ y_stride ]];
      x_ptr[ x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[ x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[ x_stride + 3] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[ x_stride ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[ y_stride + 1]];
      x_ptr[ x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[ x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[ x_stride + 7] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[ x_stride + 4 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[2 * y_stride ]];
      x_ptr[2 * x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2 * x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2 * x_stride + 3] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[2 * x_stride ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[2 * y_stride + 1]];
      x_ptr[2 * x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2 * x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2 * x_stride + 7] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[2 * x_stride + 4 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[3 * y_stride ]];
      x_ptr[3 * x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[3 * x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[3 * x_stride + 3] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[3 * x_stride ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[3 * y_stride + 1]];
      x_ptr[3 * x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[3 * x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[3 * x_stride + 7] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[3 * x_stride + 4 ] = 0;;
      x_ptr += 8;
      y_ptr += 2;
      u_ptr += 1;
      v_ptr += 1;
    }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void
yv12_to_rgbai_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
    {;
      for (x = 0; x < fixed_width; x += 2)
    {
      int rgb_y;
      int b_u0 = B_U_tab[u_ptr[0]];
      int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
      int r_v0 = R_V_tab[v_ptr[0]];
      int b_u1 = B_U_tab[u_ptr[uv_stride]];
      int g_uv1 = G_U_tab[u_ptr[uv_stride]] + G_V_tab[v_ptr[uv_stride]];
      int r_v1 = R_V_tab[v_ptr[uv_stride]];
      rgb_y = RGB_Y_tab[y_ptr[0 ]];
      x_ptr[2] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[0 ] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[3] = 0;
      rgb_y = RGB_Y_tab[y_ptr[1]];
      x_ptr[6] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[5] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[4 ] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[0 + 7] = 0;
      rgb_y = RGB_Y_tab[y_ptr[ y_stride ]];
      x_ptr[ x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[ x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[ x_stride ] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[ x_stride + 3] = 0;
      rgb_y = RGB_Y_tab[y_ptr[ y_stride + 1]];
      x_ptr[ x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[ x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[ x_stride + 4 ] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[ x_stride + 7] = 0;
      rgb_y = RGB_Y_tab[y_ptr[2 * y_stride ]];
      x_ptr[2 * x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2 * x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2 * x_stride ] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[2 * x_stride + 3] = 0;
      rgb_y = RGB_Y_tab[y_ptr[2 * y_stride + 1]];
      x_ptr[2 * x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2 * x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2 * x_stride + 4 ] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[2 * x_stride + 7] = 0;
      rgb_y = RGB_Y_tab[y_ptr[3 * y_stride ]];
      x_ptr[3 * x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[3 * x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[3 * x_stride ] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[3 * x_stride + 3] = 0;
      rgb_y = RGB_Y_tab[y_ptr[3 * y_stride + 1]];
      x_ptr[3 * x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[3 * x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[3 * x_stride + 4 ] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[3 * x_stride + 7] = 0;;
      x_ptr += 8;
      y_ptr += 2;
      u_ptr += 1;
      v_ptr += 1;
    }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void
yv12_to_argbi_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 4 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -4 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
    {;
      for (x = 0; x < fixed_width; x += 2)
    {
      int rgb_y;
      int b_u0 = B_U_tab[u_ptr[0]];
      int g_uv0 = G_U_tab[u_ptr[0]] + G_V_tab[v_ptr[0]];
      int r_v0 = R_V_tab[v_ptr[0]];
      int b_u1 = B_U_tab[u_ptr[uv_stride]];
      int g_uv1 = G_U_tab[u_ptr[uv_stride]] + G_V_tab[v_ptr[uv_stride]];
      int r_v1 = R_V_tab[v_ptr[uv_stride]];
      rgb_y = RGB_Y_tab[y_ptr[0 ]];
      x_ptr[3] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[1] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[0 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[1]];
      x_ptr[0 + 7] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[6] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[5] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[4 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[ y_stride ]];
      x_ptr[ x_stride + 3] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[ x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[ x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[ x_stride ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[ y_stride + 1]];
      x_ptr[ x_stride + 7] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[ x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[ x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[ x_stride + 4 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[2 * y_stride ]];
      x_ptr[2 * x_stride + 3] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2 * x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2 * x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[2 * x_stride ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[2 * y_stride + 1]];
      x_ptr[2 * x_stride + 7] =
        (0 >
         (((255) <
           ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y +
                          b_u0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u0) >> 13) ? (255) : ((rgb_y + b_u0) >> 13))));
      x_ptr[2 * x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y -
                           g_uv0) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv0) >> 13) ? (255) : ((rgb_y - g_uv0) >> 13))));
      x_ptr[2 * x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y +
                          r_v0) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v0) >> 13) ? (255) : ((rgb_y + r_v0) >> 13))));
      if (4 > 3)
        x_ptr[2 * x_stride + 4 ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[3 * y_stride ]];
      x_ptr[3 * x_stride + 3] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[3 * x_stride + 2] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[3 * x_stride + 1] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[3 * x_stride ] = 0;
      rgb_y = RGB_Y_tab[y_ptr[3 * y_stride + 1]];
      x_ptr[3 * x_stride + 7] =
        (0 >
         (((255) <
           ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y +
                          b_u1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + b_u1) >> 13) ? (255) : ((rgb_y + b_u1) >> 13))));
      x_ptr[3 * x_stride + 6] =
        (0 >
         (((255) <
           ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y -
                           g_uv1) >> 13))) ? 0
         : (((255) <
         ((rgb_y - g_uv1) >> 13) ? (255) : ((rgb_y - g_uv1) >> 13))));
      x_ptr[3 * x_stride + 5] =
        (0 >
         (((255) <
           ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y +
                          r_v1) >> 13))) ? 0
         : (((255) <
         ((rgb_y + r_v1) >> 13) ? (255) : ((rgb_y + r_v1) >> 13))));
      if (4 > 3)
        x_ptr[3 * x_stride + 4 ] = 0;;
      x_ptr += 8;
      y_ptr += 2;
      u_ptr += 1;
      v_ptr += 1;
    }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void
yv12_to_yuyvi_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
    {;
      for (x = 0; x < fixed_width; x += 2)
    {
      x_ptr[0 ] = y_ptr[0 ];
      x_ptr[1] = u_ptr[0 ];
      x_ptr[2] = y_ptr[1];
      x_ptr[3] = v_ptr[0 ];
      x_ptr[ x_stride ] = y_ptr[ y_stride ];
      x_ptr[ x_stride + 1] = u_ptr[ uv_stride ];
      x_ptr[ x_stride + 2] = y_ptr[ y_stride + 1];
      x_ptr[ x_stride + 3] = v_ptr[ uv_stride ];
      x_ptr[2 * x_stride ] = y_ptr[2 * y_stride ];
      x_ptr[2 * x_stride + 1] = u_ptr[0 ];
      x_ptr[2 * x_stride + 2] = y_ptr[2 * y_stride + 1];
      x_ptr[2 * x_stride + 3] = v_ptr[0 ];
      x_ptr[3 * x_stride ] = y_ptr[3 * y_stride ];
      x_ptr[3 * x_stride + 1] = u_ptr[ uv_stride ];
      x_ptr[3 * x_stride + 2] = y_ptr[3 * y_stride + 1];
      x_ptr[3 * x_stride + 3] = v_ptr[ uv_stride ];;
      x_ptr += 4;
      y_ptr += 2;
      u_ptr += 1;
      v_ptr += 1;
    }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}
void
yv12_to_uyvyi_c (uint8_t * x_ptr, int x_stride, uint8_t * y_ptr,
         uint8_t * u_ptr, uint8_t * v_ptr, int y_stride,
         int uv_stride, int width, int height, int vflip)
{
  int fixed_width = (width + 1) & ~1;
  int x_dif = x_stride - 2 * fixed_width;
  int y_dif = y_stride - fixed_width;
  int uv_dif = uv_stride - (fixed_width / 2);
  int x, y;
  if (vflip)
    {
      x_ptr += (height - 1) * x_stride;
      x_dif = -2 * fixed_width - x_stride;
      x_stride = -x_stride;
    }
  for (y = 0; y < height; y += 4)
    {;
      for (x = 0; x < fixed_width; x += 2)
    {
      x_ptr[1] = y_ptr[0 ];
      x_ptr[0 ] = u_ptr[0 ];
      x_ptr[3] = y_ptr[1];
      x_ptr[2] = v_ptr[0 ];
      x_ptr[ x_stride + 1] = y_ptr[ y_stride ];
      x_ptr[ x_stride ] = u_ptr[ uv_stride ];
      x_ptr[ x_stride + 3] = y_ptr[ y_stride + 1];
      x_ptr[ x_stride + 2] = v_ptr[ uv_stride ];
      x_ptr[2 * x_stride + 1] = y_ptr[2 * y_stride ];
      x_ptr[2 * x_stride ] = u_ptr[0 ];
      x_ptr[2 * x_stride + 3] = y_ptr[2 * y_stride + 1];
      x_ptr[2 * x_stride + 2] = v_ptr[0 ];
      x_ptr[3 * x_stride + 1] = y_ptr[3 * y_stride ];
      x_ptr[3 * x_stride ] = u_ptr[ uv_stride ];
      x_ptr[3 * x_stride + 3] = y_ptr[3 * y_stride + 1];
      x_ptr[3 * x_stride + 2] = v_ptr[ uv_stride ];;
      x_ptr += 4;
      y_ptr += 2;
      u_ptr += 1;
      v_ptr += 1;
    }
      x_ptr += x_dif + 3 * x_stride;
      y_ptr += y_dif + 3 * y_stride;
      u_ptr += uv_dif +  uv_stride;
      v_ptr += uv_dif +  uv_stride;
    }
}





void
yv12_to_yv12_c (uint8_t * y_dst, uint8_t * u_dst, uint8_t * v_dst,
        int y_dst_stride, int uv_dst_stride,
        uint8_t * y_src, uint8_t * u_src, uint8_t * v_src,
        int y_src_stride, int uv_src_stride,
        int width, int height, int vflip)
{
  int width2 = width / 2;
  int height2 = height / 2;
  int y;

  if (vflip)
    {
      y_src += (height - 1) * y_src_stride;
      u_src += (height2 - 1) * uv_src_stride;
      v_src += (height2 - 1) * uv_src_stride;
      y_src_stride = -y_src_stride;
      uv_src_stride = -uv_src_stride;
    }

  for (y = height; y; y--)
    {
      memcpy (y_dst, y_src, width);
      y_src += y_src_stride;
      y_dst += y_dst_stride;
    }

  for (y = height2; y; y--)
    {
      memcpy (u_dst, u_src, width2);
      u_src += uv_src_stride;
      u_dst += uv_dst_stride;
    }

  for (y = height2; y; y--)
    {
      memcpy (v_dst, v_src, width2);
      v_src += uv_src_stride;
      v_dst += uv_dst_stride;
    }
}

void
colorspace_init (void)
{
  int32_t i;

  for (i = 0; i < 256; i++)
    {
      RGB_Y_tab[i] = ((uint16_t) ((1.164) * (1L << 13) + 0.5)) * (i - 16);
      B_U_tab[i] = ((uint16_t) ((2.018) * (1L << 13) + 0.5)) * (i - 128);
      G_U_tab[i] = ((uint16_t) ((0.391) * (1L << 13) + 0.5)) * (i - 128);
      G_V_tab[i] = ((uint16_t) ((0.813) * (1L << 13) + 0.5)) * (i - 128);
      R_V_tab[i] = ((uint16_t) ((1.596) * (1L << 13) + 0.5)) * (i - 128);
    }
}
