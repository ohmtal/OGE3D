<?php 

//example:  php /opt/torque/OGE3D/Tools/Ohmtal/genMat.php Ground |less > materials.cs

require_once 'genMatClass.php';

$genMat = new GenMaterials();

if (isset($argv[1]))
    $genMat->materialTag = $argv[1];

echo($genMat->execute());
?>