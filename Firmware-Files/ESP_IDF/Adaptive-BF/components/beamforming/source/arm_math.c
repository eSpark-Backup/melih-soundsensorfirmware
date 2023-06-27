#include "arm_math.h"

void arm_mult_f32(
float32_t * pSrcA,
float32_t * pSrcB,
float32_t * pDst,
uint32_t blockSize)
{
    uint32_t blkCnt;                               /* Loop counter */

#if defined(ARM_MATH_NEON)
    float32x4_t vec1;
    float32x4_t vec2;
    float32x4_t res;

    /* Compute 4 outputs at a time */
    blkCnt = blockSize >> 2U;

    while (blkCnt > 0U)
    {
        /* C = A * B */

    	/* Multiply the inputs and then store the results in the destination buffer. */
        vec1 = vld1q_f32(pSrcA);
        vec2 = vld1q_f32(pSrcB);
        res = vmulq_f32(vec1, vec2);
        vst1q_f32(pDst, res);

        /* Increment pointers */
        pSrcA += 4;
        pSrcB += 4; 
        pDst += 4;
        
        /* Decrement the loop counter */
        blkCnt--;
    }

    /* Tail */
    blkCnt = blockSize & 0x3;

#else
#if defined (ARM_MATH_LOOPUNROLL)

  /* Loop unrolling: Compute 4 outputs at a time */
  blkCnt = blockSize >> 2U;

  while (blkCnt > 0U)
  {
    /* C = A * B */

    /* Multiply inputs and store result in destination buffer. */
    *pDst++ = (*pSrcA++) * (*pSrcB++);

    *pDst++ = (*pSrcA++) * (*pSrcB++);

    *pDst++ = (*pSrcA++) * (*pSrcB++);

    *pDst++ = (*pSrcA++) * (*pSrcB++);

    /* Decrement loop counter */
    blkCnt--;
  }

  /* Loop unrolling: Compute remaining outputs */
  blkCnt = blockSize % 0x4U;

#else

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_LOOPUNROLL) */
#endif /* #if defined(ARM_MATH_NEON) */

  while (blkCnt > 0U)
  {
    /* C = A * B */

    /* Multiply input and store result in destination buffer. */
    *pDst++ = (*pSrcA++) * (*pSrcB++);

    /* Decrement loop counter */
    blkCnt--;
  }

}


void arm_cmplx_mag_squared_f32(
   float32_t * pSrc,
        float32_t * pDst,
        uint32_t numSamples)
{
        uint32_t blkCnt;                               /* Loop counter */
        float32_t real, imag;                          /* Temporary input variables */

#if defined(ARM_MATH_NEON)
  float32x4x2_t vecA;
  float32x4_t vRealA;
  float32x4_t vImagA;
  float32x4_t vMagSqA;

  float32x4x2_t vecB;
  float32x4_t vRealB;
  float32x4_t vImagB;
  float32x4_t vMagSqB;

  /* Loop unrolling: Compute 8 outputs at a time */
  blkCnt = numSamples >> 3;

  while (blkCnt > 0U)
  {
    /* out = sqrt((real * real) + (imag * imag)) */

    vecA = vld2q_f32(pSrc);
    pSrc += 8;

    vRealA = vmulq_f32(vecA.val[0], vecA.val[0]);
    vImagA = vmulq_f32(vecA.val[1], vecA.val[1]);
    vMagSqA = vaddq_f32(vRealA, vImagA);

    vecB = vld2q_f32(pSrc);
    pSrc += 8;

    vRealB = vmulq_f32(vecB.val[0], vecB.val[0]);
    vImagB = vmulq_f32(vecB.val[1], vecB.val[1]);
    vMagSqB = vaddq_f32(vRealB, vImagB);

    /* Store the result in the destination buffer. */
    vst1q_f32(pDst, vMagSqA);
    pDst += 4;

    vst1q_f32(pDst, vMagSqB);
    pDst += 4;

    /* Decrement the loop counter */
    blkCnt--;
  }

  blkCnt = numSamples & 7;

#else
#if defined (ARM_MATH_LOOPUNROLL)

  /* Loop unrolling: Compute 4 outputs at a time */
  blkCnt = numSamples >> 2U;

  while (blkCnt > 0U)
  {
    /* C[0] = (A[0] * A[0] + A[1] * A[1]) */

    real = *pSrc++;
    imag = *pSrc++;
    *pDst++ = (real * real) + (imag * imag);

    real = *pSrc++;
    imag = *pSrc++;
    *pDst++ = (real * real) + (imag * imag);

    real = *pSrc++;
    imag = *pSrc++;
    *pDst++ = (real * real) + (imag * imag);

    real = *pSrc++;
    imag = *pSrc++;
    *pDst++ = (real * real) + (imag * imag);

    /* Decrement loop counter */
    blkCnt--;
  }

  /* Loop unrolling: Compute remaining outputs */
  blkCnt = numSamples % 0x4U;

#else

  /* Initialize blkCnt with number of samples */
  blkCnt = numSamples;

#endif /* #if defined (ARM_MATH_LOOPUNROLL) */
#endif /* #if defined(ARM_MATH_NEON) */

  while (blkCnt > 0U)
  {
    /* C[0] = (A[0] * A[0] + A[1] * A[1]) */

    real = *pSrc++;
    imag = *pSrc++;

    /* store result in destination buffer. */
    *pDst++ = (real * real) + (imag * imag);

    /* Decrement loop counter */
    blkCnt--;
  }

}