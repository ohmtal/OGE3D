datablock afxCameraData(DefaultCameraData)
{
   mode = "Default";
};

// $GLOBALS::useAFXCamera = true;
// $Game::DefaultCameraClass = "afxCamera";
// $Game::DefaultCameraDataBlock = DefaultCameraData;

$Game::DefaultCameraClass = Camera;
$Game::DefaultCameraDataBlock = Observer;
