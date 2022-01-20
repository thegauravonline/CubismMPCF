/*
 *  Test_ShockBubble.cpp
 *  MPCFnode
 *
 *  Created by Babak Hejazialhosseini  on 6/16/11.
 *  Copyright 2011 ETH Zurich. All rights reserved.
 *
 */
#include <limits>
#include <sstream>

#ifdef _USE_NUMA_
#include <numa.h>
#include <omp.h>
#endif

#include <Profiler.h>

#include "Test_SIC.h"
#include "Tests.h"

static Real heaviside_smooth_local(const Real phi, const Real eps)
{
    const Real alpha = M_PI*min(1., max(0., (phi+0.5*eps)/eps));
    return 0.5+0.5*cos(alpha);
}

void Test_SIC::_ic(FluidGrid& grid)
{
	if (VERBOSITY > 0)
		cout << "SIC Initial condition..." ;
    
	vector<BlockInfo> vInfo = grid.getBlocksInfo();
    
    const double G1 = Simulation_Environment::GAMMA1-1;
    const double G2 = Simulation_Environment::GAMMA2-1;
    const double F1 = Simulation_Environment::GAMMA1*Simulation_Environment::PC1;
    const double F2 = Simulation_Environment::GAMMA2*Simulation_Environment::PC2;
    
    const double h = vInfo.front().h_gridpoint;
    
#pragma omp parallel
	{
#ifdef _USE_NUMA_
		const int cores_per_node = numa_num_configured_cpus() / numa_num_configured_nodes();
		const int mynode = omp_get_thread_num() / cores_per_node;
		numa_run_on_node(mynode);
#endif
        
#pragma omp for
		for(int i=0; i<(int)vInfo.size(); i++)
		{
            BlockInfo info = vInfo[i];
            FluidBlock& b = *(FluidBlock*)info.ptrBlock;
            
            for(int iz=0; iz<FluidBlock::sizeZ; iz++)
                for(int iy=0; iy<FluidBlock::sizeY; iy++)
                    for(int ix=0; ix<FluidBlock::sizeX; ix++)
                    {
                        Real p[3], post_shock[3];
                        info.pos(p, ix, iy, iz);
                        
                        const double r1 = sqrt(pow(p[0]-bubble_pos[0],2)+pow(p[1]-bubble_pos[1],2)+pow(p[2]-bubble_pos[2],2));
                        const double r2 = sqrt(pow(p[0]-bubble_pos[0]-0.05,2)+pow(p[1]-bubble_pos[1]-0.35,2)+pow(p[2]-bubble_pos[2],2));
                        
                        const double bubble = Simulation_Environment::heaviside_smooth(min(HUGE_VAL+r1-radius, r2-0.7*radius));
                        
                        const double bubble_p = Simulation_Environment::heaviside_smooth(r2-0.7*radius-4*Simulation_Environment::EPSILON);//Simulation_Environment::heaviside_smooth(r-radius);//Simulation_Environment::heaviside_smooth(r-radius);
                        
                        //const double shock_pressure = 3530;
                        
                        const Real pre_shock[3] = {1000,0,100};
                        
                        b(ix, iy, iz).rho      = 1.0*bubble+pre_shock[0]*(1-bubble);
                        b(ix, iy, iz).u        = 0;
                        b(ix, iy, iz).v        = 0;
                        b(ix, iy, iz).w        = 0;
                        
                        const double pressure  = 0.0234*bubble+pre_shock[2]*(1-bubble);
                        
                        SETUP_MARKERS_IC
						
						assert(!isnan(b(ix, iy, iz).P));
						assert(!isinf(b(ix, iy, iz).P));
						assert(b(ix, iy, iz).P >= 1);
						
						//b(ix, iy, iz).P = max(b(ix, iy, iz).P, numeric_limits<Real>::epsilon());
 						assert(isnormal(b(ix, iy, iz).P));
                        
                        //**************************
                        //Let's do 1D in Colonius
                        //**************************
                         /*Real p[3];
                         info.pos(p, ix, iy, iz);
                         
                         //test 5.3 equation 32
                         //const Real pre_shock[3] = {10,0,10};
                         //const Real post_shock[3] = {0.125,0,0.1};
                         //const double bubble = Simulation_Environment::heaviside(0.5-p[0]);
                         
                         //test 5.3 equation 31
                         const Real pre_shock[3] = {1.241,0,2.753};
                         const Real post_shock[3] = {0.991,0,3.059e-4};
                         const double bubble = Simulation_Environment::heaviside(0.5-p[0]);
                         
                         const double shock = 1-bubble;
                         b(ix, iy, iz).rho      =  shock*pre_shock[0] + (1-shock)*post_shock[0];
                         b(ix, iy, iz).u        = pre_shock[1]*b(ix, iy, iz).rho*shock;
                         b(ix, iy, iz).v        = 0;
                         b(ix, iy, iz).w        = 0;
                         
                         const double pressure  = pre_shock[2]*shock+post_shock[2]*(1-shock);
                         
                         SETUP_MARKERS_IC*/
                    }
        }
	}
	
	if (VERBOSITY > 0)
		cout << "done." << endl;
}

void Test_SIC::setup()
{
    printf("////////////////////////////////////////////////////////////\n");
    printf("////////////             TEST SIC            ///////////////\n");
    printf("////////////////////////////////////////////////////////////\n");
    
    _setup_constants();
    
    parser.mute();
    
    if (parser("-morton").asBool(0))
        grid = new GridMorton<FluidGrid>(BPDX, BPDY, BPDZ);
    else
        grid = new FluidGrid(BPDX, BPDY, BPDZ);
    
    assert(grid != NULL);
    
    stepper = new FlowStep_LSRK3(*grid, CFL, Simulation_Environment::GAMMA1, Simulation_Environment::GAMMA2, parser, VERBOSITY, &profiler, Simulation_Environment::PC1, Simulation_Environment::PC2, bAWK);
    
    if(bRESTART)
    {
        _restart();
        _dump("restartedcondition.vti");
    }
    else
        _ic(*grid);
}
