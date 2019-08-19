//
//  main.cpp
//  Projections
//
//  Created by Martin Prantl on 09/07/2018.
//  Copyright © 2018 Martin Prantl. All rights reserved.
//

#include <chrono>
#include <iostream>
#include <array>
#include <vector>

#include "ProjectionInfo.h"
#include "./Projections/Mercator.h"
#include "./Projections/Miller.h"
#include "ProjectionRenderer.h"
#include "lodepng.h"

#include "./simd/ProjectionInfo_simd.h"
#include "./simd/Projections/Miller_simd.h"
#include "./simd/Projections/Mercator_simd.h"
#include "./simd/MapProjectionUtils_simd.h"

using namespace Projections;

namespace ns = Projections::Simd;

int main(int argc, const char * argv[]) {
    
    unsigned w = 800;
    unsigned h = 600;
    
    /*
    std::array<Projections::Pixel<int>, 8> p;
    ns::Mercator mercSimd;
    ns::Miller millerSimd;
    mercSimd.ProjectInverse(p);
    mercSimd.GetFrameWidth();
    
    Reprojection reprojectionSimd = Projections::Simd::ProjectionUtils::CreateReprojection(&millerSimd, &mercSimd);
    */
    
    std::vector<uint8_t> imgRawData;
    //imgRawData.resize(800 * 600, 0);
    
    std::vector<uint8_t> fileData;
    lodepng::load_file(fileData, "D://image_eu.png");
    
    lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_GREY);
    
    //===================================================
    //Build input projection
    //===================================================
    
    Coordinate bbMin, bbMax;
    bbMin.lat = -71.52_deg; bbMin.lon = -20.0_deg;
    bbMax.lat = 23.1_deg; bbMax.lon = 44.1_deg;
    
    //create input projection and set its visible frame
    Miller * miller = new Miller();
    miller->SetFrame(bbMin, bbMax, w, h, false);
    
    ns::Miller * millerSimd = new ns::Miller();
    millerSimd->SetFrame(bbMin, bbMax, w, h, false);
    
    //render image in input projection - it should take input image and
    //render it 1:1 based on ipImage frame
    ProjectionRenderer pd(miller);
    //pd.AddBorders("E://hranice//ll.csv");
    pd.DrawImage(&imgRawData[0], ProjectionRenderer::GREY,  w, h, miller);
    //pd.DrawBorders();
    pd.SaveToFile("D://miller.png");
    
    
    //===================================================
    // Build output projection
    //===================================================
    
    //Mercator latitude in <-85, 85>
    if (bbMin.lat.deg() < -85) bbMin.lat = -85.0_deg;
    if (bbMax.lat.deg() > 85) bbMax.lat = 85.0_deg;
    
    Mercator * mercator = new Mercator();
    mercator->SetFrame(bbMin, bbMax, w, h, false);
    
    pd.SetProjection(mercator);
    
    auto t00 = std::chrono::high_resolution_clock::now();
    
    //compute mapping from input -> output projection
    //newData[index] = oldData[reprojection[index]]
    Reprojection reprojection = Projections::ProjectionUtils::CreateReprojection(miller, mercator);
    //Reprojection reprojection = ns::ProjectionUtils::CreateReprojection(miller, mercator);
    
    auto tc = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t00).count();
    printf("%lld ms\n", tc);


	pd.Clear();
	pd.DrawImage(&imgRawData[0], ProjectionRenderer::GREY, reprojection);
	pd.DrawBorders();
	pd.SaveToFile("D://mercator_float.png");

	//============================================================================

    ns::Mercator * mercatorSimd = new ns::Mercator();
    mercatorSimd->SetFrame(bbMin, bbMax, w, h, false);
    
    
    
    Coordinate cc;
    cc.lat = 10.52_deg; cc.lon = -5.0_deg;
    
    auto res = miller->Project<float>(cc);
    
    std::array<Projections::Coordinate, 8> c;
    for (int i = 0; i < 8; i++) {
        c[i] = cc;
    }
    auto res2 = millerSimd->Project<float>(c);
    
	t00 = std::chrono::high_resolution_clock::now();
    Reprojection reprojectionSimd = ns::ProjectionUtils::CreateReprojection(millerSimd, mercatorSimd);    
	tc = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t00).count();
	printf("%lld ms\n", tc);

    pd.DrawImage(&imgRawData[0], ProjectionRenderer::GREY, reprojectionSimd);    
    pd.DrawBorders();
    pd.SaveToFile("D://mercator_float_avx.png");
    	
    
    return 0;
}
