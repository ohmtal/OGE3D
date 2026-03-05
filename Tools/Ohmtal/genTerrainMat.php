<?php 
require_once 'genMatClass.php';



$genMat = new GenMaterials();
$genMat->genTerrainMaterial = true;

if (isset($argv[1]))
    $genMat->materialTag = $argv[1];
else
    $genMat->materialTag = "TerrainMat";

echo($genMat->execute());
?>