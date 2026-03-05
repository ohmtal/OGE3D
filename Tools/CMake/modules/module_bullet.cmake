
# Bullet module

# XXTH changed to ON and OFF and ON again  and added GMK but OFF make the engine crazy ... simtime get out of sync ?! 
# XXTH i dont really need it and it cause more trouble than i want !! 2025-03-29
option(TORQUE_PHYSICS_BULLET "Use Bullet physics" OFF)
option(TORQUE_PHYSICS_GMK "Use GMK physics require Bullet!" OFF)

if( NOT TORQUE_PHYSICS_BULLET )
    return()
endif()
	           
addDef( "TORQUE_PHYSICS_BULLET" )
addDef( "TORQUE_PHYSICS_ENABLED" )

addPath( "${srcDir}/T3D/physics/bullet" )
addInclude( "${libDir}/bullet/src" )

addLib( "libbullet" )

# XXTH  added
if (TORQUE_PHYSICS_GMK)
 addDef( "GMK_ENABLED" )
 addPath("${srcDir}/T3D/logickingMechanics")
 addPath("${srcDir}/T3D/logickingMechanics/physics")
 addPath("${srcDir}/T3D/logickingMechanics/physics/bullet")
endif()


	
