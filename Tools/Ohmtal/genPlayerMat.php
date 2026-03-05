<?php 
require_once 'genMatClass.php';

$genMat = new GenMaterials();
$genMat->castDynamicShadows = true;

if (isset($argv[1]))
    $genMat->materialTag = $argv[1];
else
    $genMat->materialTag = "PlayerMat";

echo($genMat->execute());
?>