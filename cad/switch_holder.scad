$fn=20;

module hole() {
    translate([0,0,-1]) rotate([0,0,90]) cylinder(d=4, h=6);
}

difference() {
    union() {
        cube([42,18,1]);
        translate([0,0,1])
            cube([26,8,3]);
        translate([31,0,1])
            cube([11,18,3]);
    }
    translate([3.5,3.5,0]) hole();
    translate([22.5,3.5,0]) hole();
    translate([37.5,3.5,0]) hole();
    translate([37.5,14.5,0]) hole();
}