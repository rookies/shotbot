//rotate([0,90,0]) cylinder(d=23.7, h=30, center=true);
difference() {
/* holder: */
translate([-5,0,0])
rotate([0,90,0])
linear_extrude(10)
difference() {
    // -z,y
    union() {
        translate([-1.65,0]) square([27,43.7], center=true);
    }
    for (i = [-1,1]) {
        translate([-10,i*18]) square([40,8], center=true);
    }
    for (i = [0:20]) {
        translate([i,0]) circle(d=24);
    }
}
/* screw holes: */
for (i = [-1,1]) {
    translate([0,i*18,-15]) cylinder(d=3, h=10, $fn=20);
}
}