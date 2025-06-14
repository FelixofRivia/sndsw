#include "Scifi.h"
#include "ScifiPoint.h"

#include "TGeoManager.h"
#include "FairRun.h"                    // for FairRun
#include "FairRuntimeDb.h"              // for FairRuntimeDb
#include "TList.h"                      // for TListIter, TList (ptr only)
#include "TObjArray.h"                  // for TObjArray
#include "TString.h"                    // for TString

#include "TClonesArray.h"
#include "TVirtualMC.h"

#include "TGeoBBox.h"
#include "TGeoTrd1.h"
#include "TGeoCompositeShape.h"
#include "TGeoTube.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TParticle.h"

#include "FairVolume.h"
#include "FairGeoVolume.h"
#include "FairGeoNode.h"
#include "FairRootManager.h"
#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
#include "FairGeoMedia.h"
#include "FairGeoBuilder.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include "ShipDetectorList.h"
#include "ShipUnit.h"
#include "ShipStack.h"

#include "TGeoUniformMagField.h"
#include <stddef.h>                     // for NULL
#include <iostream>                     // for operator<<, basic_ostream, etc

using std::cout;
using std::endl;

using namespace ShipUnit;

Scifi::Scifi()
: FairDetector("Scifi", "", kTRUE),
fTrackID(-1),
fVolumeID(-1),
fPos(),
fMom(),
fTime(-1.),
fLength(-1.),
fELoss(-1),
eventHeader(0),
last_run_time(-1),
last_run_pos(-1),
last_time_alignment_tag(""),
last_position_alignment_tag(""),
alignment_init(false),
fScifiPointCollection(new TClonesArray("ScifiPoint"))
{
}

Scifi::Scifi(const char* name, Bool_t Active, const char* Title)
: FairDetector(name, true, kLHCScifi),
fTrackID(-1),
fVolumeID(-1),
fPos(),
fMom(),
fTime(-1.),
fLength(-1.),
fELoss(-1),
eventHeader(0),
last_run_time(-1),
last_run_pos(-1),
last_time_alignment_tag(""),
last_position_alignment_tag(""),
alignment_init(false),
fScifiPointCollection(new TClonesArray("ScifiPoint"))
{
}

Scifi::~Scifi()
{
    if (fScifiPointCollection) {
        fScifiPointCollection->Delete();
        delete fScifiPointCollection;
    }
}

void Scifi::Initialize()
{
    FairDetector::Initialize();
}

// -----   Private method InitMedium
Int_t Scifi::InitMedium(const char* name)
{
    static FairGeoLoader *geoLoad=FairGeoLoader::Instance();
    static FairGeoInterface *geoFace=geoLoad->getGeoInterface();
    static FairGeoMedia *media=geoFace->getMedia();
    static FairGeoBuilder *geoBuild=geoLoad->getGeoBuilder();
    
    FairGeoMedium *ShipMedium=media->getMedium(name);
    
    if (!ShipMedium)
    {
        Fatal("InitMedium","Material %s not defined in media file.", name);
        return -1111;
    }
    TGeoMedium* medium=gGeoManager->GetMedium(name);
    if (medium!=NULL)
        return ShipMedium->getMediumIndex();
    return geoBuild->createMedium(ShipMedium);
}

void Scifi::ConstructGeometry()
{
  InitMedium("CarbonComposite");
  TGeoMedium *CarbonComposite = gGeoManager->GetMedium("CarbonComposite");

  InitMedium("rohacell");
  TGeoMedium *rohacell = gGeoManager->GetMedium("rohacell");

  InitMedium("air");
  TGeoMedium *air = gGeoManager->GetMedium("air");

  InitMedium("Polycarbonate");
  TGeoMedium *PlasticBase = gGeoManager->GetMedium("Polycarbonate");
  
  InitMedium("Polystyrene");
  TGeoMedium *Polystyrene = gGeoManager->GetMedium("Polystyrene");

  InitMedium("PMMA");
  TGeoMedium *PMMA = gGeoManager->GetMedium("PMMA");

  InitMedium("PMMA2");
  TGeoMedium *PMMA2 = gGeoManager->GetMedium("PMMA2");
  
  InitMedium("Epoxy");
  TGeoMedium *Epoxy = gGeoManager->GetMedium("Epoxy");

  TGeoVolume *volTarget = gGeoManager->GetVolume("volTarget");
  
  InitMedium("iron");
  TGeoMedium *Fe =gGeoManager->GetMedium("iron");

// external parameters
	Double_t fXDimension = conf_floats["Scifi/xdim"];
	Double_t fYDimension = conf_floats["Scifi/ydim"];
	Double_t fZDimension = conf_floats["Scifi/zdim"];

	Double_t fWidthScifiMat   = conf_floats["Scifi/scifimat_width"];
	Double_t fLengthScifiMat = conf_floats["Scifi/scifimat_length"];
	Double_t fZScifiMat              = conf_floats["Scifi/scifimat_z"];
	Double_t fZEpoxyMat          = conf_floats["Scifi/epoxymat_z"];
	Double_t fGapScifiMat        = conf_floats["Scifi/scifimat_gap"]; //dead zone between mats

	Double_t fFiberLength         = conf_floats["Scifi/fiber_length"];
	Double_t fScintCore_rmax  =  conf_floats["Scifi/scintcore_rmax"];
	Double_t fClad1_rmin  =  conf_floats["Scifi/clad1_rmin"];
	Double_t fClad1_rmax =  conf_floats["Scifi/clad1_rmax"];
	Double_t fClad2_rmin  =  conf_floats["Scifi/clad2_rmin"];
	Double_t fClad2_rmax =  conf_floats["Scifi/clad2_rmax"];

	Double_t fHorPitch         =  conf_floats["Scifi/horizontal_pitch"]; //Fiber position params
	Double_t fVertPitch        =  conf_floats["Scifi/vertical_pitch"];
	Double_t fOffsetRowS  =  conf_floats["Scifi/rowlong_offset"];
	Double_t fOffsetRowL  =  conf_floats["Scifi/rowshort_offset"];

	Double_t fZCarbonFiber = conf_floats["Scifi/carbonfiber_z"];
	Double_t fZHoneycomb = conf_floats["Scifi/honeycomb_z"];
	Double_t fZGlue = conf_floats["Scifi/glue_z"];
	Double_t fZAirgap = conf_floats["Scifi/airgap_z"];

	Double_t fXPlastBar = conf_floats["Scifi/plastbar_x"]; //Dimension of plastic bar
	Double_t fYPlastBar = conf_floats["Scifi/plastbar_y"]; 
	Double_t fZPlastBar = conf_floats["Scifi/plastbar_z"];
	
	// Air gaps in baby SciFi modules
	Double_t fZBabyPlaneGap = conf_floats["Scifi/plane_gap"];
	Double_t fZBabyTedlarToPlaneGap = conf_floats["Scifi/tedlar_to_plane"];
	// Offsets between the upstream tedlar sheet and the upstream passive block
	// It includes the air gap formed btw the baby module frame and the upstream tedlar sheet. 
	Double_t fStationOffset[4] = {0, conf_floats["Scifi/station_offset1"],
	 	 	 	         conf_floats["Scifi/station_offset2"],
	 	 	 	         conf_floats["Scifi/station_offset3"]};

	Int_t fNFibers_Srow = conf_ints["Scifi/nfibers_shortrow"]; 
	Int_t fNFibers_Lrow = conf_ints["Scifi/nfibers_longrow"]; 
	Int_t fNFibers_z = conf_ints["Scifi/nfibers_z"]; 

	Double_t fSeparationBrick = conf_floats["Scifi/scifi_separation"];  //Separation between successive SciFi volumes
	Double_t fZOffset = conf_floats["Scifi/offset_z"]; 
	Int_t fNMats   = conf_ints["Scifi/nmats"];             //Number of mats in one SciFi plane
	Int_t fNScifi    = conf_ints["Scifi/nscifi"];               //Number of Scifi walls
	Int_t fNSiPMs  = conf_ints["Scifi/nsipm_mat"]; //Number of SiPMs per SciFi mat

	Double_t fWidthChannel = conf_floats["Scifi/channel_width"]; //One channel width 
	Double_t fHeightChannel = conf_floats["Scifi/channel_height"]; //One channel height
	Double_t fCharr = conf_floats["Scifi/charr_width"]; //Width of an array of 64 channels without gaps
	Double_t fEdge = conf_floats["Scifi/sipm_edge"]; //Edge at the left and right sides of the SiPM
	Double_t fCharrGap = conf_floats["Scifi/charr_gap"]; //Gap between two charr
	Double_t fBigGap = conf_floats["Scifi/sipm_diegap"]; //Gap between two arrays
	Int_t fNSiPMChan = conf_ints["Scifi/nsipm_channels"]; //Number of channels in each SiPM
	Double_t firstChannelX = conf_floats["Scifi/firstChannelX"]; //local X Position of first channel in plane
	
	Int_t PassiveBlockNotCenterred = conf_ints["Scifi/PassiveBlocknotCenterred"]; // flag showing whether the passive material (FeBlocks) are to be centerred wrt the Scifi

//edge positions in TI18 survey system:
        std::map<int,TVector3> Vedges;
        Vedges[0]=TVector3(-conf_floats["Scifi/Xpos0"],conf_floats["Scifi/Zpos0"],conf_floats["Scifi/Ypos0"]);
        Vedges[1]=TVector3(-conf_floats["Scifi/Xpos1"],conf_floats["Scifi/Zpos1"],conf_floats["Scifi/Ypos1"]);
        Vedges[2]=TVector3(-conf_floats["Scifi/Xpos2"],conf_floats["Scifi/Zpos2"],conf_floats["Scifi/Ypos2"]);
        Vedges[3]=TVector3(-conf_floats["Scifi/Xpos3"],conf_floats["Scifi/Zpos3"],conf_floats["Scifi/Ypos3"]);
        Vedges[4]=TVector3(-conf_floats["Scifi/Xpos4"],conf_floats["Scifi/Zpos4"],conf_floats["Scifi/Ypos4"]);

//edge position in Scifi engineering drawing, y down, z towards IP1, pos X left.
//                                                         need y up, z away from IP1, pos X left:  y and z need to change sign.

        TVector3 Sedge = TVector3(conf_floats["Scifi/EdgeAX"],-conf_floats["Scifi/EdgeAY"],-conf_floats["Scifi/EdgeAZ"]);
//first channel position in Scifi engineering drawing:
        TVector3 SHfirst = TVector3(conf_floats["Scifi/FirstChannelHX"],-conf_floats["Scifi/FirstChannelHY"],-conf_floats["Scifi/FirstChannelHZ"]);
        TVector3 SVfirst = TVector3(conf_floats["Scifi/FirstChannelVX"],-conf_floats["Scifi/FirstChannelVY"],-conf_floats["Scifi/FirstChannelVZ"]);

//first channel position in sndsw local plane:
        TVector3 LHfirst = TVector3(conf_floats["Scifi/LfirstChannelHX"],conf_floats["Scifi/LfirstChannelHY"],conf_floats["Scifi/LfirstChannelHZ"]);
        TVector3 LVfirst = TVector3(conf_floats["Scifi/LfirstChannelVX"],conf_floats["Scifi/LfirstChannelVY"],conf_floats["Scifi/LfirstChannelVZ"]);
// moving plane to match edges:
        std::map<int,TVector3> DeltasH;
        std::map<int,TVector3> DeltasV;
        for (int i=0;i<5;i++){
            DeltasH[i]  = Vedges[i] - LHfirst + SHfirst - Sedge;
            DeltasV[i]  = Vedges[i] - LVfirst + SVfirst - Sedge;
        }
   Double_t totalThickness{};

  //Carbon Fiber Film
  TGeoVolume *CarbonFiberVolume = gGeoManager->MakeBox("CarbonFiber", CarbonComposite, fXDimension/2, fYDimension/2, fZCarbonFiber/2);
  CarbonFiberVolume->SetLineColor(kGray - 2);
  CarbonFiberVolume->SetVisibility(1);

  //Honeycomb Rohacell
  TGeoVolume *HoneycombVolume = gGeoManager->MakeBox("Honeycomb", rohacell, fXDimension/2, fYDimension/2, fZHoneycomb/2);
  HoneycombVolume->SetLineColor(kYellow);
  HoneycombVolume->SetVisibility(1);

  // Glue between Carbon Fiber and Honeycomb or Glue between Carbon Fiber and Scifi mat
  TGeoVolume *GlueVolume = gGeoManager->MakeBox("Glue", Epoxy, fXDimension/2, fYDimension/2, fZGlue/2);
  GlueVolume->SetLineColor(kYellow-1);
  GlueVolume->SetVisibility(1);

  // Air gap in the middle
  TGeoVolume *AirGapVolume = gGeoManager->MakeBox("Airgap", air, fXDimension/2, fYDimension/2, fZAirgap/2);
  AirGapVolume->SetLineColor(kGray-1);
  AirGapVolume->SetVisibility(1);

  //Plastic/Air
  //Definition of the box containing polycarbonate pieces and an air gap
  TGeoVolume *PlasticAirVolume = gGeoManager->MakeBox("PlasticAir", air, fXDimension/2, fYDimension/2, fZPlastBar/2); 
  PlasticAirVolume->SetLineColor(kGray-1);
  PlasticAirVolume->SetVisibility(1);
  PlasticAirVolume->SetVisDaughters(1);
  
  //Plastic bars
  TGeoVolume *PlasticBarVolume = gGeoManager->MakeBox("PlasticBar", PlasticBase, fXPlastBar/2, fYPlastBar/2, fZPlastBar/2); 
  PlasticBarVolume->SetLineColor(kGray-4);
  PlasticBarVolume->SetVisibility(1);

  PlasticAirVolume->AddNode(PlasticBarVolume, 0, new TGeoTranslation(- fXDimension/2 + fXPlastBar/2, 0, 0));  //bars are placed || to y
  PlasticAirVolume->AddNode(PlasticBarVolume, 1, new TGeoTranslation(+ fXDimension/2 - fXPlastBar/2, 0, 0));

  //Plastic Glue  Air
  //Definition of the box containing glue between plastic bar and scifi mat, and an air in the middel
  TGeoVolume *PlasticGlueAirVolume = gGeoManager->MakeBox("PlasticGlueAir", air, fXDimension/2, fYDimension/2, fZGlue/2); 
  PlasticGlueAirVolume->SetLineColor(kGray-1);
  PlasticGlueAirVolume->SetVisibility(1);
  PlasticGlueAirVolume->SetVisDaughters(1);
  
  //Plastic glue bars
  TGeoVolume *PlasticGlueBarVolume = gGeoManager->MakeBox("PlasticGlueBar", Epoxy, fXPlastBar/2, fYPlastBar/2, fZGlue/2); 
  PlasticGlueBarVolume->SetLineColor(kYellow-1);
  PlasticGlueBarVolume->SetVisibility(1);

  PlasticGlueAirVolume->AddNode(PlasticGlueBarVolume, 0, new TGeoTranslation(- fXDimension/2 + fXPlastBar/2, 0, 0));  //bars are placed || to y
  PlasticGlueAirVolume->AddNode(PlasticGlueBarVolume, 1, new TGeoTranslation(+ fXDimension/2 - fXPlastBar/2, 0, 0));

  //Definition of the two air gaps for baby SciFi
  TGeoVolume *PlaneGapVolume = gGeoManager->MakeBox("PlaneGap", air, fXDimension/2, fYDimension/2, fZBabyPlaneGap/2);
  PlaneGapVolume->SetLineColor(kOrange-4);
  PlaneGapVolume->SetTransparency(50);
  PlaneGapVolume->SetVisibility(1);
  TGeoVolume *TedlarToPlaneGapVolume = gGeoManager->MakeBox("TedlarToPlaneGap", air, fXDimension/2, fYDimension/2, fZBabyTedlarToPlaneGap/2);
  TedlarToPlaneGapVolume->SetLineColor(kOrange-4);
  TedlarToPlaneGapVolume->SetTransparency(50);
  TedlarToPlaneGapVolume->SetVisibility(1);

  //Fiber volume that contains the scintillating core and double cladding
  TGeoVolumeAssembly *FiberVolume = new TGeoVolumeAssembly("FiberVolume");

  TGeoVolume *ScintCoreVol = gGeoManager->MakeTube("ScintCoreVol", Polystyrene, 0, fScintCore_rmax, fFiberLength/2); 
  TGeoVolume *Clad1Vol = gGeoManager->MakeTube("Clad1Vol", PMMA, fClad1_rmin, fClad1_rmax, fFiberLength/2); 
  TGeoVolume *Clad2Vol = gGeoManager->MakeTube("Clad2Vol", PMMA2, fClad2_rmin, fClad2_rmax, fFiberLength/2); 
  
  FiberVolume->AddNode(ScintCoreVol, 0);
  FiberVolume->AddNode(Clad1Vol, 0);
  FiberVolume->AddNode(Clad2Vol, 0);
  FiberVolume->SetVisDaughters(kFALSE);

  //Add SciFi fiber as sensitive unit
  AddSensitiveVolume(ScintCoreVol);

  //Fiber and plane rotations
  TGeoRotation *rothorfiber = new TGeoRotation("rothorfiber", 90, 90, 0);
  TGeoRotation *rotvertfiber = new TGeoRotation("rotvertfiber", 0, 90, 0);
  TGeoRotation *rot = new TGeoRotation("rot", 90, 180, 0);

  //SciFi mats for X and Y fiber planes
  Double_t MatThickness{};
  TGeoMedium *MatMaterial;
  if (fNScifi==5){ //TI18 SciFi
     MatMaterial = Epoxy;  
     MatThickness = fZEpoxyMat;
  }
  if (fNScifi==4){ // no glue in baby SciFi planes
     MatMaterial = air;  
     MatThickness = fZScifiMat;
  }
  TGeoVolume *HorMatVolume  = gGeoManager->MakeBox("HorMatVolume", MatMaterial, fLengthScifiMat/2, fWidthScifiMat/2, MatThickness/2); 
  TGeoVolume *VertMatVolume = gGeoManager->MakeBox("VertMatVolume", MatMaterial, fWidthScifiMat/2, fLengthScifiMat/2, MatThickness/2); 
 
  Double_t zPosM;
  Double_t offsetS =  -fWidthScifiMat/2 + fOffsetRowS;
  Double_t offsetL =  -fWidthScifiMat/2 + fOffsetRowL;

  // All fibres will be assigned station number 1 and mat number 1, to keep compatibility with the STMRFFF format.
  int dummy_station = 1;
  int dummy_mat = 1;
  
  //Adding horizontal fibers
  for (int irow = 0; irow < fNFibers_z; irow++){
    zPosM =  -fZScifiMat/2 + fClad2_rmax + irow*fVertPitch;
    if (irow%2 == 0){
      for (int ifiber = 0; ifiber < fNFibers_Srow; ifiber++){
	HorMatVolume->AddNode(FiberVolume, 1e6*dummy_station + 1e5*0 + 1e4*dummy_mat + 1e3*(irow + 1) + ifiber + 1, new TGeoCombiTrans("rottranshor0", 0, offsetS + ifiber*fHorPitch, zPosM, rothorfiber));
      }
    }
    else{
      for (int ifiber = 0; ifiber < fNFibers_Lrow; ifiber++){
	HorMatVolume->AddNode(FiberVolume, 1e6*dummy_station + 1e5*0 + 1e4*dummy_mat + 1e3*(irow + 1) + ifiber + 1, new TGeoCombiTrans("rottranshor1", 0, offsetL + ifiber*fHorPitch, zPosM, rothorfiber));
      }
    }
  }
  
  //Adding vertical fibers
  for (int irow = 0; irow < fNFibers_z; irow++){
    zPosM =  -fZScifiMat/2 + fClad2_rmax + irow*fVertPitch;
    if (irow%2 == 0){
      for (int ifiber = 0; ifiber < fNFibers_Srow; ifiber++){
	VertMatVolume->AddNode(FiberVolume, 1e6*dummy_station + 1e5*1 + 1e4*dummy_mat +  1e3*(irow + 1) + ifiber + 1, new TGeoCombiTrans("rottransvert0", offsetS + ifiber*fHorPitch, 0, zPosM, rotvertfiber));
      }
    }
    else{
      for (int ifiber = 0; ifiber < fNFibers_Lrow; ifiber++){
	VertMatVolume->AddNode(FiberVolume, 1e6*dummy_station + 1e5*1 + 1e4*dummy_mat + 1e3*(irow + 1) + ifiber + 1, new TGeoCombiTrans("rottransvert1", offsetL + ifiber*fHorPitch, 0, zPosM, rotvertfiber));
      }
    }
  }

  // for testbeam 2023
  // for now the distinct feature of the testbeam could be the 4 SciFi planes
  std::map<int, TGeoVolume*> volFeTarget;
  std::map<int, float> fFeTargetX;
  std::map<int, float> fFeTargetY;
  std::map<int, float> fFeTargetZ;
  for ( int i = 0; i < fNScifi; i++){
      std::string station = std::to_string(i+1);
      fFeTargetX[i] = conf_floats[TString("Scifi/FeTargetX"+station)];
      fFeTargetY[i] = conf_floats[TString("Scifi/FeTargetZ"+station)];
      fFeTargetZ[i] = conf_floats[TString("Scifi/FeTargetY"+station)];
  }

  // DetID is of the form: 
  // first digit - station number
  // second digit - type of the plane: 0-horizontal fiber, 1-vertical fiber
  // third digit - mat number
  // fourth digit - row number (in Z direction)
  // last three digits - fiber number
  // e.g. DetID = 1021074 -> station 1, horizontal fiber plane, mat 2, row 1, fiber 74
  for (int istation = 0; istation < fNScifi; istation++){
    Int_t node = 1e6*(istation+1);
    std::string station = std::to_string(istation+1);
    TGeoVolumeAssembly *ScifiVolume        = new TGeoVolumeAssembly( TString("ScifiVolume"+station) );
    TGeoVolumeAssembly *ScifiHorPlaneVol = new TGeoVolumeAssembly( TString("ScifiHorPlaneVol"+station) );
    TGeoVolumeAssembly *ScifiVertPlaneVol = new TGeoVolumeAssembly( TString("ScifiVertPlaneVol"+station) );

    //TI18 SciFi
    if (fNScifi==5){
	//Adding the first half of the SciFi module that contains horizontal fibres
	ScifiVolume->AddNode(CarbonFiberVolume, 0, new TGeoTranslation(0, 0, fZCarbonFiber/2));
	ScifiVolume->AddNode(GlueVolume, 0, new TGeoTranslation(0, 0, fZCarbonFiber + fZGlue/2));
	ScifiVolume->AddNode(HoneycombVolume, 0, new TGeoTranslation(0, 0, fZCarbonFiber + fZGlue + fZHoneycomb/2));
	ScifiVolume->AddNode(GlueVolume, 1, new TGeoTranslation(0, 0, fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue/2));
	ScifiVolume->AddNode(CarbonFiberVolume, 1, new TGeoTranslation(0, 0, fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue + fZCarbonFiber/2));
	ScifiVolume->AddNode(GlueVolume, 2, new TGeoTranslation(0, 0, fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue + fZCarbonFiber + fZGlue/2));
	ScifiVolume->AddNode(ScifiHorPlaneVol, node, new TGeoTranslation(0, 0, fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue + fZCarbonFiber + fZGlue + fZEpoxyMat/2));
	ScifiVolume->AddNode(PlasticGlueAirVolume, 0, new TGeoTranslation(0, 0, fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue + fZCarbonFiber + fZGlue + fZEpoxyMat + fZGlue/2));
	ScifiVolume->AddNode(PlasticAirVolume, 0, new TGeoTranslation(0, 0, fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue + fZCarbonFiber + fZGlue + fZEpoxyMat + fZGlue + fZPlastBar/2));

	Double_t first_half_z = fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue + fZCarbonFiber + fZGlue + fZEpoxyMat + fZGlue + fZPlastBar;
	ScifiVolume->AddNode(AirGapVolume, 0, new TGeoTranslation(0, 0, first_half_z + fZAirgap/2));

	//Adding the second half of the SciFi module that contains vertical fibres
	ScifiVolume->AddNode(PlasticAirVolume, 1, new TGeoCombiTrans("rottrans0", 0, 0, first_half_z + fZAirgap + fZPlastBar/2, rot));
	ScifiVolume->AddNode(PlasticGlueAirVolume, 1, new TGeoCombiTrans("rottrans0",0, 0, first_half_z + fZAirgap + fZPlastBar + fZGlue/2, rot));
	ScifiVolume->AddNode(ScifiVertPlaneVol, node, new TGeoTranslation(0, 0, first_half_z + fZAirgap + fZPlastBar + fZGlue + fZEpoxyMat/2));
	ScifiVolume->AddNode(GlueVolume, 3, new TGeoTranslation(0, 0, first_half_z + fZAirgap + fZPlastBar + fZGlue + fZEpoxyMat + fZGlue/2));
	ScifiVolume->AddNode(CarbonFiberVolume, 2, new TGeoTranslation(0, 0, first_half_z + fZAirgap + fZPlastBar + fZGlue + fZEpoxyMat + fZGlue + fZCarbonFiber/2));
	ScifiVolume->AddNode(GlueVolume, 4, new TGeoTranslation(0, 0, first_half_z + fZAirgap + fZPlastBar + fZGlue + fZEpoxyMat + fZGlue + fZCarbonFiber + fZGlue/2));
	ScifiVolume->AddNode(HoneycombVolume, 0, new TGeoTranslation(0, 0, first_half_z + fZAirgap + fZPlastBar + fZGlue + fZEpoxyMat + fZGlue + fZCarbonFiber + fZGlue + fZHoneycomb/2));
	ScifiVolume->AddNode(GlueVolume, 5, new TGeoTranslation(0, 0, first_half_z + fZAirgap + fZPlastBar + fZGlue + fZEpoxyMat + fZGlue + fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue/2));
	ScifiVolume->AddNode(CarbonFiberVolume, 3, new TGeoTranslation(0, 0, first_half_z + fZAirgap + fZPlastBar + fZGlue + fZEpoxyMat + fZGlue + fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue + fZCarbonFiber/2));

	// Double_t totalThickness = fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue + fZCarbonFiber + fZGlue + fZEpoxyMat + fZGlue + fZPlastBar + fZAirgap + fZPlastBar + fZGlue + fZEpoxyMat + fZGlue + fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue + fZCarbonFiber;
	totalThickness = fZCarbonFiber + fZGlue + fZHoneycomb + fZGlue + fZCarbonFiber +
                          fZGlue + fZEpoxyMat + fZGlue + fZPlastBar + fZAirgap + fZPlastBar +
                          fZGlue + fZEpoxyMat + fZGlue + fZCarbonFiber + fZGlue + fZHoneycomb +
                          fZGlue + fZCarbonFiber;
    }

    //Baby SciFi
    // No glue in this case and one uses fZScifiMat and not the fZEpoxyMat for the mat thickness
    if (fNScifi==4){
       //Adding the first half of the SciFi module that contains horizontal fibres
       ScifiVolume->AddNode(TedlarToPlaneGapVolume, 1, new TGeoTranslation(0, 0, fZBabyTedlarToPlaneGap/2));
       ScifiVolume->AddNode(ScifiHorPlaneVol, node, new TGeoTranslation(0, 0, fZBabyTedlarToPlaneGap + fZScifiMat/2));

       // Add the air gap between the X and Y plane
       ScifiVolume->AddNode(PlaneGapVolume, 0, new TGeoTranslation(0, 0, fZBabyTedlarToPlaneGap + fZScifiMat + fZBabyPlaneGap/2));

       //Adding the second half of the SciFi module that contains vertical fibres
       ScifiVolume->AddNode(ScifiVertPlaneVol, node, new TGeoTranslation(0, 0, fZBabyTedlarToPlaneGap + 3*fZScifiMat/2 + fZBabyPlaneGap));
       ScifiVolume->AddNode(TedlarToPlaneGapVolume, 2, new TGeoTranslation(0, 0, 3*fZBabyTedlarToPlaneGap/2 + 2*fZScifiMat + fZBabyPlaneGap));
       
       totalThickness = 2*fZBabyTedlarToPlaneGap + 2*fZScifiMat + fZBabyPlaneGap;
    }

    volTarget->AddNode(ScifiVolume, node, 
               new TGeoTranslation(DeltasV[istation][0], DeltasH[istation][1], DeltasH[istation][2]));

    // std::cout<<"deltas "<<DeltasV[istation][0]<<" "<< DeltasH[istation][1]<<" "<< DeltasH[istation][2]<<" "<<totalThickness<<std::endl;
    // for 2023 testbeam put iron blocks of variable length in between SciFi planes - the planes are dowstream of the blocks!
    // for the 2024 testbeam, the iron blocks are aligned to Brick 1 and laid on the concrete (same as SciFi 1)
    if (fNScifi==4 && istation != 0) {
       volFeTarget[istation] = gGeoManager->MakeBox(TString("volFeTarget"+station),Fe,fFeTargetX[istation]/2., fFeTargetY[istation]/2., fFeTargetZ[istation]/2.);
       volFeTarget[istation]->SetLineColor(kGreen-4);
       volTarget->AddNode(volFeTarget[istation],1,
                                         new TGeoTranslation(DeltasV[istation][0] - PassiveBlockNotCenterred*fabs(fXDimension-fFeTargetX[istation])/2.,
                                                             DeltasH[istation][1] + PassiveBlockNotCenterred*(DeltasH[0][1]-DeltasH[istation][1]
                                                                                                             +fabs(fYDimension-fFeTargetY[istation])/2.),
                                                             DeltasH[istation][2] - fStationOffset[istation] - fFeTargetZ[istation]/2.));
    }

    //Creating Scifi planes by appending fiber mats
    for (int imat = 0; imat < fNMats; imat++){
      int N = fNMats==1 ? imat : imat-1;
      
      //Placing mats along Y 
      ScifiHorPlaneVol->AddNode(HorMatVolume, 1e6*(istation+1) + 1e4*(imat + 1), new TGeoTranslation(0, N*(fWidthScifiMat+fGapScifiMat), 0));
        
      //Placing mats along X
      ScifiVertPlaneVol->AddNode(VertMatVolume, 1e6*(istation+1) + 1e5 + 1e4*(imat + 1), new TGeoTranslation(N*(fWidthScifiMat+fGapScifiMat), 0, 0)); 
    }
  }

}

void Scifi::SiPMOverlap()
{   
    if (gGeoManager->FindVolumeFast("SiPMmapVol")){return;}
	Double_t fLengthScifiMat = conf_floats["Scifi/scifimat_length"];
	Double_t fWidthChannel = conf_floats["Scifi/channel_width"];
	Double_t fHeightChannel = conf_floats["Scifi/channel_height"]; 
	Int_t fNSiPMChan = conf_ints["Scifi/nsipm_channels"];
	Int_t fNSiPMs  = conf_ints["Scifi/nsipm_mat"];
	Int_t fNMats   = conf_ints["Scifi/nmats"]; 
	Double_t fEdge = conf_floats["Scifi/sipm_edge"];
	Double_t fCharr = conf_floats["Scifi/charr_width"];
	Double_t fCharrGap = conf_floats["Scifi/charr_gap"];
	Double_t fBigGap = conf_floats["Scifi/sipm_diegap"];
	Double_t firstChannelX = conf_floats["Scifi/firstChannelX"];

    //Contains all plane SiPMs, defined for horizontal fiber plane
    //To obtain SiPM map for vertical fiber plane rotate by 90 degrees around Z
    TGeoVolumeAssembly *SiPMmapVol = new TGeoVolumeAssembly("SiPMmapVol");

    TGeoVolume*ChannelVol = gGeoManager->MakeBox("ChannelVol", 0, fLengthScifiMat/2, fWidthChannel/2, fHeightChannel/2);

    //DetID for each channel:
    //first digit: mat number (0-2)
    //second digit: SiPM number (0-3)
    //last three digits: channel number (0-127)

    Double_t SiPMArray_fullwidth = fEdge+fCharr+fCharrGap+fCharr+fEdge;
    TGeoVolumeAssembly *SiPMArrayVol;
    int N = fNMats == 1 ? 1 : 0;
    Double_t pos = -fEdge+firstChannelX + N*fLengthScifiMat;
    for (int imat = 0; imat < fNMats; imat++){
      for (int isipms = 0; isipms < fNSiPMs; isipms++){
        pos+= fEdge;
        for (int ichannel = 0; ichannel < fNSiPMChan; ichannel++){
            SiPMmapVol->AddNode(ChannelVol, imat*10000+isipms *1000 + ichannel, new TGeoTranslation(0, pos, 0));
            pos += fWidthChannel;
            if (ichannel==(fNSiPMChan/2-1)){pos += fCharrGap;}
        }
        pos+=fEdge+fBigGap;
    }
   }
}

void Scifi::InitEvent(SNDLHCEventHeader *e){
  // get mapping to eventHeader
  eventHeader = e;

  // Initialize
  if (not alignment_init) {
    alignment_init = true;
    // Get available tags from the geometry file
    std::string tag_string;
    
    // Time alignment tags
    for (auto key : conf_floats){
      tag_string = key.first.Data();
      if (tag_string.find("Scifi/station1t_") != std::string::npos){
	covered_runs_time_alignment.push_back(stoi(tag_string.substr(tag_string.find("t_")+2)));
      }
    }
    std::sort(covered_runs_time_alignment.begin(),covered_runs_time_alignment.end());
    // Position alignment tags
    for (auto key : conf_floats){
      tag_string = key.first.Data();
      if (tag_string.find("Scifi/LocM100t_") != std::string::npos){
	covered_runs_position_alignment.push_back(stoi(tag_string.substr(tag_string.find("t_")+2)));
      }
    }
    std::sort(covered_runs_position_alignment.begin(),covered_runs_position_alignment.end());
  }
};

Bool_t  Scifi::ProcessHits(FairVolume* vol)
{
	/** This method is called from the MC stepping */
	//Set parameters at entrance of volume. Reset ELoss.
	if ( gMC->IsTrackEntering() ) 
	{
		fELoss  = 0.;
		fTime   = gMC->TrackTime() * 1.0e09;
		fLength = gMC->TrackLength();
		gMC->TrackPosition(fPos);
		gMC->TrackMomentum(fMom);
		TGeoNavigator* nav = gGeoManager->GetCurrentNavigator();

		int fibre_local_id = nav->GetMother()->GetNumber() - 1e6 - 1e4; // Local ID within the mat.
		int fibre_mat_id = nav->GetMother(2)->GetNumber(); // Get mat ID.

		int fibre_station_number = int( fibre_mat_id / 1e6); // Get the station number from the mat

		int fibre_mat_id_station_removed = fibre_mat_id - fibre_station_number*1e6;
		int fibre_mat_number = int((fibre_mat_id_station_removed - int(fibre_mat_id_station_removed/1e5)*1e5)/1e4);

		fVolumeID = fibre_local_id + fibre_station_number*1e6 + fibre_mat_number*1e4;

		if (fVolumeID==0){std::cout<<"fiber vol id "<<nav->GetMother()->GetName()<<std::endl;}

	}
	// Sum energy loss for all steps in the active volume
	fELoss += gMC->Edep();

	// Create ScifiPoint at exit of active volume
	if ( gMC->IsTrackExiting()    ||
			gMC->IsTrackStop()       || 
			gMC->IsTrackDisappeared()   ) {
		if (fELoss == 0. ) { return kFALSE; }
		fTrackID  = gMC->GetStack()->GetCurrentTrackNumber();
/* STMRFFF
First digit S: station # within the sub-detector
Second digit T: type of the plane: 0-horizontal fiber plane, 1-vertical fiber plane
Third digit M: determines the mat number
Fourth digit R: row number (in Z direction)
Last three digits F: fiber number
*/
		TParticle* p=gMC->GetStack()->GetCurrentTrack();
		Int_t pdgCode = p->GetPdgCode();

		TLorentzVector Pos; 
		gMC->TrackPosition(Pos); 
		Double_t xmean = (fPos.X()+Pos.X())/2. ;
		Double_t ymean = (fPos.Y()+Pos.Y())/2. ;
		Double_t zmean = (fPos.Z()+Pos.Z())/2. ;

		AddHit(fTrackID,fVolumeID, TVector3(xmean, ymean,  zmean),
				TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime, fLength,
				fELoss, pdgCode);

		// Increment number of det points in TParticle
		ShipStack* stack = (ShipStack*) gMC->GetStack();
		stack->AddPoint(kLHCScifi);
	}   

	return kTRUE;
}

Double_t Scifi::GetCorrectedTime(Int_t fDetectorID, Double_t rawTime, Double_t L){
/* expect time in u.ns  and  path length to sipm u.cm */
	TString tag = "t";
	TString sID;

	if (eventHeader){
		Int_t fRunNumber = eventHeader->GetRunId();
		if (fRunNumber != last_run_time){
		  last_run_time = fRunNumber;
		  if (fRunNumber<1) {
		  	LOG(ERROR) << "Scifi::GetCorrectedTime: non valid run number "<<fRunNumber;
		  	return rawTime;
		  }
		  
		  if (covered_runs_time_alignment.size()!=0){
		      tag = "t_"+std::to_string(covered_runs_time_alignment[covered_runs_time_alignment.size()-1]);
		      for (int i=1; i<covered_runs_time_alignment.size(); i++){
		           if (fRunNumber>=covered_runs_time_alignment[i-1] && fRunNumber<covered_runs_time_alignment[i]){
		               tag = "t_"+std::to_string(covered_runs_time_alignment[i-1]);
		           }
		      }
		      // special case
		      if (fRunNumber<5193 && fRunNumber>5174) tag = "t_"+std::to_string(covered_runs_time_alignment[0]);
		  }
		  else{		
		       // allow reading older geo files with letter tags i.e. A, B, C
		      tag = "tA";
		      if (fRunNumber>5116 && !(fRunNumber<5193 && fRunNumber>5174) ) {tag = "tB";}
		  }
		  // 2023 and 2024 testbeam data don't have custom tags
		  if (fRunNumber>=1e5) {tag = "t";}
		  last_time_alignment_tag = tag;
		}
	}
	sID.Form("%i",fDetectorID);
	Double_t cor = conf_floats["Scifi/station"+TString(sID(0,1))+last_time_alignment_tag];
	if (sID(1,1)=="0"){
	        // In the teatbeam 2024, SciFi 2H needs internal time corrections per channel
		if (conf_ints["Scifi/channelTimeAlignment"]==1 &&  floor(fDetectorID/100000)==20) {
		   cor+=conf_vectors["Scifi/station"+TString(sID(0,4))+"XXX"+last_time_alignment_tag][fDetectorID%1000];
		}
		cor+=conf_floats["Scifi/station"+TString(sID(0,1))+"H"+TString(sID(2,1))+last_time_alignment_tag];
	}
	else{
		cor+=conf_floats["Scifi/station"+TString(sID(0,1))+"V"+TString(sID(2,1))+last_time_alignment_tag];
	}
	cor += L/conf_floats["Scifi/signalSpeed"];
	return rawTime-cor;
}

void Scifi::GetPosition(Int_t fDetectorID, TVector3& A, TVector3& B) 
{
//	TGeoVolumeAssembly *SiPMmapVol = gGeoManager->FindVolumeFast("SiPMmapVol");
//	if(!SiPMmapVol ){SiPMmapVol=SiPMOverlap();}

/* STMRFFF
 First digit S: 			station # within the sub-detector
 Second digit T: 		type of the plane: 0-horizontal fiber plane, 1-vertical fiber plane
 Third digit M: 			determines the mat number
 Fourth digit R: 		row number (in Z direction)
 Last three digits F: 	fiber number
*/

	Int_t station_number = int(fDetectorID/1e6);
	Int_t mat_number = int(fDetectorID/1e4)%int(fDetectorID/1e5);

	Int_t local_fibre_id = fDetectorID - (station_number-1)*1e6 - (mat_number-1)*1e4;
	TString sLocalID;
	sLocalID.Form("%i", local_fibre_id);

	TString sID;
	sID.Form("%i",fDetectorID);
	TString path = "/cave_1/Detector_0/volTarget_1/ScifiVolume"+TString(sID(0,1))+"_"+TString(sID(0,1))+"000000/";
	if (sID(1,1)=="0"){
		path+="ScifiHorPlaneVol"+TString(sID(0,1))+"_"+TString(sID(0,1))+"000000/";
		path+="HorMatVolume_"+TString(sID(0,3))+"0000/";
	}else{
		path+="ScifiVertPlaneVol"+TString(sID(0,1))+"_"+TString(sID(0,1))+"000000/";
		path+="VertMatVolume_"+TString(sID(0,3))+"0000/";
	}
	path+="FiberVolume_"+sLocalID;
	TGeoNavigator* nav = gGeoManager->GetCurrentNavigator();
	nav->cd(path);
	LOG(DEBUG) <<path<<" "<<fDetectorID;
	TGeoNode* W = nav->GetCurrentNode();
	TGeoBBox* S = dynamic_cast<TGeoBBox*>(W->GetVolume()->GetShape());

	Double_t top[3] = {0,0,S->GetDZ()};
	Double_t bot[3] = {0,0,-(S->GetDZ())};
	Double_t Gtop[3],Gbot[3];
	nav->LocalToMaster(top, Gtop);   nav->LocalToMaster(bot, Gbot);
	A.SetXYZ(Gtop[0],Gtop[1],Gtop[2]);
	B.SetXYZ(Gbot[0],Gbot[1],Gbot[2]);

}
TVector3 Scifi::GetLocalPos(Int_t id, TVector3* glob){
	TString sID;
	sID.Form("%i",id);
	TString path = "/cave_1/Detector_0/volTarget_1/ScifiVolume"+TString(sID(0,1))+"_"+TString(sID(0,1))+"000000/";
	if (sID(1,1)=="0"){
		path+="ScifiHorPlaneVol"+TString(sID(0,1))+"_"+TString(sID(0,1))+"000000";
	}else{
		path+="ScifiVertPlaneVol"+TString(sID(0,1))+"_"+TString(sID(0,1))+"000000";
	}
	TGeoNavigator* nav = gGeoManager->GetCurrentNavigator();
	nav->cd(path);
	Double_t aglob[3];
	Double_t aloc[3];
	glob->GetXYZ(aglob);
	nav->MasterToLocal(aglob,aloc);
	return TVector3(aloc[0],aloc[1],aloc[2]);
}

void Scifi::GetSiPMPosition(Int_t SiPMChan, TVector3& A, TVector3& B) 
{
/* STMRFFF
 First digit S: 		station # within the sub-detector
 Second digit T: 		type of the plane: 0-horizontal fiber plane, 1-vertical fiber plane
 Third digit M: 		determines the mat number 0-2
 Fourth digit S: 		SiPM number  0-3
 Last three digits F: 	local SiPM channel number in one mat  0-127
*/
	Int_t locNumber            = SiPMChan%100000;
	Int_t globNumber         = int(SiPMChan/100000)*100000;
	Float_t locPosition        = SiPMPos[locNumber]; // local position in plane of reference plane.
	Double_t fFiberLength  = conf_floats["Scifi/fiber_length"];
	Int_t fNMats   = conf_ints["Scifi/nmats"]; 
	
	TString tag = "";

	// in case of old data with FairEventHeader, user will be responsible to use the correct geofile.
	if (eventHeader){
		Int_t fRunNumber = eventHeader->GetRunId();
		if (fRunNumber != last_run_pos){
		  last_run_pos = fRunNumber;

		  if (fRunNumber<1) {
		  LOG(ERROR) << "Scifi::GetSiPMPosition: non valid run number "<<fRunNumber;
		  return;
		  }
		  
		  if (covered_runs_position_alignment.size()!=0){
		      tag = "t_"+std::to_string(covered_runs_position_alignment[covered_runs_position_alignment.size()-1]);
		      for (int i=1; i<covered_runs_position_alignment.size(); i++){
		           if (fRunNumber>=covered_runs_position_alignment[i-1] && fRunNumber<covered_runs_position_alignment[i]){
		               tag = "t_"+std::to_string(covered_runs_position_alignment[i-1]);
		           }
		      }
		  }
		  else{
		      // allow reading older geo files with letter tags i.e. A, B, C
		      tag = "E";
		      if (fRunNumber<4575) {tag = "A";}
		      else if (fRunNumber<4855) {tag = "B";}
		      else if (fRunNumber<5172) {tag = "C";}
		      else if (fRunNumber<5431) {tag = "D";}
		  }
		  // 2023 testbeam data doesn't have a custom tag
		  if (fRunNumber>=1e5) {tag = "";}
		  last_position_alignment_tag = tag;
		}
	}
	TString sID;
	sID.Form("%i",SiPMChan);
	Int_t digits = fNMats==1 ? 2 : 1; 
	locPosition += conf_floats["Scifi/LocM"+TString(sID(0,3))+last_position_alignment_tag];
	Float_t rotPhi = conf_floats["Scifi/RotPhiS"+TString(sID(0,digits))+last_position_alignment_tag];
	Float_t rotPsi = conf_floats["Scifi/RotPsiS"+TString(sID(0,digits))+last_position_alignment_tag];
	Float_t rotTheta = conf_floats["Scifi/RotThetaS"+TString(sID(0,digits))+last_position_alignment_tag];

	Double_t loc[3] = {0,0,0};
	TString path = "/cave_1/Detector_0/volTarget_1/ScifiVolume"+TString(sID(0,1))+"_"+TString(sID(0,1))+"000000/";
	TGeoNavigator* nav = gGeoManager->GetCurrentNavigator();
	Double_t glob[3] = {0,0,0};

	if (sID(1,1)=="0"){
		path+="ScifiHorPlaneVol"+TString(sID(0,1))+"_"+TString(sID(0,1))+"000000";
		loc[0] = -fFiberLength/2 - (rotPhi + rotPsi)*locPosition ;
		loc[1] = locPosition - fFiberLength/2 * (rotPhi + rotPsi) ;
		loc[2] = rotTheta*locPosition;
		nav->cd(path);
		nav->LocalToMaster(loc, glob);
		A.SetXYZ( glob[0], glob[1],glob[2] );
		loc[0] = fFiberLength/2 - (rotPhi + rotPsi)*locPosition ;
		loc[1] = locPosition + fFiberLength/2 * (rotPhi + rotPsi) ;
		loc[2] = - rotTheta*locPosition;
		nav->LocalToMaster(loc, glob);
		B.SetXYZ( glob[0], glob[1],glob[2] );
	}else{
		path+="ScifiVertPlaneVol"+TString(sID(0,1))+"_"+TString(sID(0,1))+"000000";
		loc[0] = locPosition + fFiberLength/2*(rotPhi + rotPsi);
		loc[1] = -fFiberLength/2 + locPosition*(rotPhi + rotPsi);
		loc[2] = -fFiberLength/2*rotTheta;
		nav->cd(path);
		nav->LocalToMaster(loc, glob);
		A.SetXYZ( glob[0], glob[1],glob[2] );
		loc[0] = locPosition - fFiberLength/2*(rotPhi + rotPsi);
		loc[1] = fFiberLength/2 + locPosition*(rotPhi + rotPsi);
		loc[2] = -fFiberLength/2*rotTheta;
		nav->LocalToMaster(loc, glob);
		B.SetXYZ( glob[0], glob[1],glob[2] );
	}
}

Double_t Scifi::ycross(Double_t a,Double_t R,Double_t x)
{
	Double_t y = -1;
	Double_t A = R*R - (x-a)*(x-a);
	if ( !(A<0) ){y = TMath::Sqrt(A);}
	return y;
}
Double_t Scifi::integralSqrt(Double_t ynorm)
{
	Double_t y = 1./2.*(ynorm*TMath::Sqrt(1-ynorm*ynorm)+TMath::ASin(ynorm));
	return y;
}
Double_t Scifi::fraction(Double_t R,Double_t x,Double_t y)
{
	Double_t F= 2*R*R*(integralSqrt(y/R) );
	F-=(2*x*y);
	Double_t result = F/(R*R*TMath::Pi());
       return  result;
}
Double_t Scifi::area(Double_t a,Double_t R,Double_t xL,Double_t xR)
{
	Double_t fracL = -1;
	Double_t fracR = -1;
	if (xL<=a-R && xR>=a+R) {return 1;}
	Double_t leftC    = ycross(a,R,xL);
	Double_t rightC = ycross(a,R,xR);
	if (leftC<0 && rightC<0) {return -1;}
	if ( !(rightC<0) ){  fracR = fraction(R,abs(xR-a),rightC);}
	if ( !(leftC<0) )   {   fracL = fraction(R,abs(xL-a),leftC);}
	Double_t theAnswer = 0;
	if ( !(leftC<0) ) { 
		if(xL<a){theAnswer += 1-fracL;}
		else{      theAnswer += fracL;}
		if ( !(rightC<0) ) {theAnswer -=1;}
	}
	if ( !(rightC<0) ){
		if(xR>a){ theAnswer += 1-fracR;}
		else{      theAnswer +=  fracR;}
	}
	return theAnswer;
}

void Scifi::SiPMmapping(){
	Float_t fibresRadius = -1;
	Float_t dSiPM = -1;
	TGeoNode* vol;
	TGeoNode* fibre;
	SiPMOverlap();           // 12 SiPMs per mat, made for horizontal mats, fibres staggered along y-axis.
	auto sipm    = gGeoManager->FindVolumeFast("SiPMmapVol");
	TObjArray* Nodes = sipm->GetNodes();
	auto plane  = gGeoManager->FindVolumeFast("ScifiHorPlaneVol1");
	for (int imat = 0; imat < plane->GetNodes()->GetEntriesFast(); imat++){
		auto mat =  static_cast<TGeoNode*>(plane->GetNodes()->At(imat));
		Float_t t1 = mat->GetMatrix()->GetTranslation()[1];
		auto vmat = mat->GetVolume();
		for (int ifibre = 0; ifibre < vmat->GetNodes()->GetEntriesFast(); ifibre++){
			fibre = static_cast<TGeoNode*>(vmat->GetNodes()->At(ifibre));
			if  (fibresRadius<0){
				auto tmp = fibre->GetVolume()->GetShape();
				auto S = dynamic_cast<TGeoBBox*>(tmp);
				fibresRadius = S->GetDX();
			}
			Float_t t2 = fibre->GetMatrix()->GetTranslation()[1];
			Int_t fID = fibre->GetNumber()%100000 + imat*1e4;     // local fibre number, global fibre number = SO+fID
			Float_t a = t1+t2;

	//  check for overlap with any of the SiPM channels in the same mat
			for(Int_t nChan = 0; nChan< Nodes->GetEntriesFast();nChan++){        // 12 SiPMs total and 4 SiPMs per mat times 128 channels
				vol = static_cast<TGeoNode*>(Nodes->At(nChan));
				Int_t N = vol->GetNumber()%100000;
				if (imat!=int(N/10000)){continue;}
				Float_t xcentre = vol->GetMatrix()->GetTranslation()[1];
				if  (dSiPM<0){
					TGeoBBox* B = dynamic_cast<TGeoBBox*>(vol->GetVolume()->GetShape());
					dSiPM = B->GetDY();
				}
				if (TMath::Abs(xcentre-a)>4*fibresRadius){ continue;} // no need to check further
				Float_t W = area(a,fibresRadius,xcentre-dSiPM,xcentre+dSiPM);
				if (W<0){ continue;}
				std::array<float, 2> Wa;
				Wa[0] = W;
				Wa[1] = a;
				fibresSiPM[N][fID] = Wa;
			}
		}
	}
  // calculate also local SiPM positions based on fibre positions and their fraction
  // probably an overkill, maximum difference between weighted average and central position < 6 micron.
	std::map<Int_t,std::map<Int_t,std::array<float, 2>>>::iterator it;
	std::map<Int_t,std::array<float, 2>>::iterator itx;
	for (it = fibresSiPM.begin(); it != fibresSiPM.end(); it++)
	{
		Int_t N = it->first;
		Float_t m = 0;
		Float_t w = 0;
		for (itx = it->second.begin(); itx != it->second.end(); itx++)
		{
			m+=(itx->second)[0]*(itx->second)[1];
			w+=(itx->second)[0];
		}
		SiPMPos[N]=m/w;
	}
// make inverse mapping, which fibre is associated to which SiPMs
	for (it = fibresSiPM.begin(); it != fibresSiPM.end(); it++)
	{
		Int_t N = it->first;
		for (itx = it->second.begin(); itx != it->second.end(); itx++)
		{
			Int_t nfibre = itx->first;
			siPMFibres[nfibre][N]=itx->second;
		}
	}
}
void Scifi::EndOfEvent()
{
    fScifiPointCollection->Clear();
}


void Scifi::Register()
{
    
    /** This will create a branch in the output tree called
     ScifiPoint, setting the last parameter to kFALSE means:
     this collection will not be written to the file, it will exist
     only during the simulation.
     */
    
    FairRootManager::Instance()->Register("ScifiPoint", "Scifi",
                                          fScifiPointCollection, kTRUE);
}

TClonesArray* Scifi::GetCollection(Int_t iColl) const
{
    if (iColl == 0) { return fScifiPointCollection; }
    else { return NULL; }
}

void Scifi::Reset()
{
    fScifiPointCollection->Clear();
}


ScifiPoint* Scifi::AddHit(Int_t trackID, Int_t detID,
                           TVector3 pos, TVector3 mom,
                           Double_t time, Double_t length,
                           Double_t eLoss, Int_t pdgCode)
{
    TClonesArray& clref = *fScifiPointCollection;
    Int_t size = clref.GetEntriesFast();
    return new(clref[size]) ScifiPoint(trackID, detID, pos, mom,
                                       time, length, eLoss, pdgCode);
}

ClassImp(Scifi)
