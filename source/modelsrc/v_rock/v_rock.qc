/*
==============================================================================

QC script generated by Half-Life MDL Decompiler 1.2 
2003, Kratisto. Based on code from Valve's HL SDK.

v_rock.mdl

Original internal name:
"u:\hl1\models\dmc\view_models\v_dmc_gl\v_rock.mdl"

==============================================================================
*/

$modelname "v_rock.mdl"
$cd ".\"
$cdtexture ".\"
$scale 1.0
$cliptotextures


$bbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$cbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$eyeposition 0.000000 0.000000 0.000000


//reference mesh(es)
$body "studio" "v_gl_ref"


// 1 attachment(s)
$attachment 0 "flash" 0.000000 0.000000 0.000000

// 3 hit box(es)
$hbox 0 "Cylinder04" -1.960000 -1.960000 -5.780371 1.960000 1.960000 12.900000
$hbox 0 "muzzle" -1.830000 -1.320000 -1.079075 1.830000 1.830000 6.770000
$hbox 0 "barrel" -1.220000 0.000000 0.000000 1.220000 1.220000 13.680000

// 2 animation sequence(s)
$sequence "idle" "idle" fps 30 
$sequence "shoot" "shoot" fps 40 { event 5001 3 "24" } 

// End of QC script.
