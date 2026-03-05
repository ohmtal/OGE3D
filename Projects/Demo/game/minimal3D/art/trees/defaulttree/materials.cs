//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

singleton Material(defaultTree_bark_material)
{
	mapTo = "defaulttree_bark-material";

	diffuseMap[0] = $defaultGame @ "/art/trees/defaulttree/defaulttree_bark_diffuse.dds";
	normalMap[0] = $defaultGame @ "/art/trees/defaulttree/defaulttree_bark_normal_specular.dds";
	specularMap[0] = "";

	diffuseColor[0] = "1 1 1 1";
	specular[0] = "0.9 0.9 0.9 1";
	specularPower[0] = 10;

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
   pixelSpecular[0] = "1";
   materialTag0 = "Vegetation";
};

singleton Material(defaulttree_material)
{
	mapTo = "defaulttree-material";

	diffuseMap[0] = $defaultGame @ "/art/trees/defaulttree/defaulttree_diffuse_transparency.dds";
	normalMap[0] = $defaultGame @ "/art/trees/defaulttree/defaulttree_normal_specular.dds";
	specularMap[0] = "";

	diffuseColor[0] = "1 1 1 1";
	specular[0] = "0.9 0.9 0.9 1";
	specularPower[0] = 10;

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
   pixelSpecular[0] = "1";
   alphaTest = "1";
   alphaRef = "127";
   materialTag0 = "Vegetation";
};

singleton Material(defaultTree_fronds_material)
{
   mapTo = "defaulttree_fronds-material";
   diffuseMap[0] = $defaultGame @ "/art/trees/defaulttree/defaulttree_frond_diffuse_transparency.dds";
   normalMap[0] = $defaultGame @ "/art/trees/defaulttree/defaulttree_frond_normal_specular.dds";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   pixelSpecular[0] = "1";
   translucentBlendOp = "None";
   alphaTest = "1";
   alphaRef = "50";
   translucent = "1";
   materialTag0 = "Vegetation";
   specular[2] = "1 1 1 1";
   castDynamicShadows = "1";
};
//--- defaulttree.dae MATERIALS BEGIN ---
singleton Material(defaulttree_ColorEffectR27G177B88_material)
{
	mapTo = "ColorEffectR27G177B88-material";

	diffuseMap[0] = "";
	normalMap[0] = "";
	specularMap[0] = "";

	diffuseColor[0] = "0.105882 0.694118 0.345098 1";
	specular[0] = "1 1 1 1";
	specularPower[0] = 10;

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
};

//--- defaulttree.dae MATERIALS END ---

