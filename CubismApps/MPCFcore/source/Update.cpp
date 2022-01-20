/*
 *  Update.cpp
 *  MPCFcore
 *
 *  Created by Babak Hejazialhosseini  on 6/9/11.
 *  Copyright 2011 ETH Zurich. All rights reserved.
 *
 */

#include <cassert>
#include <iostream>
#include <cstdlib>
#include <limits>

#include "common.h"
#include "Update.h"

void Update_CPP::compute(const Real * const src, Real * const dst, const int gptfloats) const
{
	assert(gptfloats >= 7);
	
	const int N = _BLOCKSIZE_ * _BLOCKSIZE_ * _BLOCKSIZE_ * gptfloats;
	
	for(int i=0; i<N; i+=gptfloats)
    {
        for(int comp = 0; comp < gptfloats; comp++)
        {
            //if (isnan(dst[i+comp]))
              //  printf("isnan component before update is %d\n", comp);
            
            //if (isnan(src[i+comp]))
              //  printf("src isnan component after update is %d, i=%d\n", comp, i);
            
            dst[i+comp] += m_b * src[i+comp];
            
            assert(!isnan(src[i+comp]));
            assert(!isnan(dst[i+comp]));
        }
        //assert(dst[i+0]>0);
        //assert(dst[i+4]>0);
    }
}
