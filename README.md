
Sample:

    //===================================================
	//Build input projection
	//===================================================

	IProjectionInfo::Coordinate bbMin, bbMax;
	bbMin.lat = -90.0_deg; bbMin.lon = -180.0_deg;	
	bbMax.lat = 90.0_deg; bbMax.lon = 180.0_deg;
	
	//create input projection and set its visible frame
	IProjectionInfo * equirect = new Equirectangular();
	equirect->SetFrame(bbMin, bbMax, w, h, false);

	//render image in input projection - it should take input image and
	//render it 1:1 based on ipImage frame
	ProjectionRenderer pd(equirect);
	pd.AddBorders("E://hranice//ll.csv");
	pd.DrawImage(&img.rawData[0], w, h, equirect);
	pd.DrawBorders();
	pd.SaveToFile("equi.png");


	//===================================================
	// Build output projection
	//===================================================

    //Mercator latitude in <-85, 85>
    if (bbMin.lat.deg() < -85) bbMin.lat = -85.0_deg;			
	if (bbMax.lat.deg() > 85) bbMax.lat = 85.0_deg;
	
	IProjectionInfo * mercator = new Mercator();	
	mercator->SetFrame(bbMin, bbMax, w, h, false);
								
	pd.SetProjection(mercator);
	
	//compute mapping from input -> output projection
	//newData[index] = oldData[reprojection[index]]
	IProjectionInfo::Reprojection reprojection = IProjection::CreateReprojection(equirect, mercator);
	pd.DrawImage(&img.rawData[0], reprojection);

	pd.DrawBorders();
	pd.SaveToFile("mercator.png");

	
