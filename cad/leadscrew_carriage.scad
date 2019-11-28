$fn=20;
SD=16; /* screw distance */
SHD=2; /* screw hole diameter */
SHDP=10; /* screw hole depth */
W=18; /* width */
LSHD=12; /* lead screw hole diameter */
D=30; /* depth */
H=40; /* height */
HO=29; /* height offset bottom / mid leadscrew */

module screwHole() {
    translate([-1,D/2,0]) rotate([0,90,0]) cylinder(d=SHD, h=SHDP+1);
}

module leadScrewHoleWithScrews() {
    translate([-1,D/2,0])
        rotate([0,90,0])
        cylinder(d=LSHD, h=W+2);
    translate([0,-SD/2,0]) screwHole();
    translate([0,+SD/2,0]) screwHole();
    translate([0,0,-SD/2]) screwHole();
    translate([0,0,+SD/2]) screwHole();
}

difference() {
    cube([W,D,H]);
    translate([0,0,HO])
        leadScrewHoleWithScrews();
}