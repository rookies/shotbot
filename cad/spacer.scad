H=15;

$fn=20;
difference() {
    translate([0,0,-H/2]) linear_extrude(H) minkowski() {
        square([55,13], center=true);
        circle(2);
    }
    for (i = [-1,1]) {
        translate([i*21,0,0])
            cylinder(d=4,h=H+2,center=true);
    }
}