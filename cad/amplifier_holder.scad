$fn=20;

difference() {
    union() {
        cube([35,25,2]);
        cube([35,2,20]);
    }
    /* potentiometer hole: */
    translate([7.5,0,13]) rotate([90,0,0]) cylinder(d=7.5, h=4.5, center=true);
    /* screw holes & countersinks: */
    translate([12.5,14.5,0]) cylinder(d1=2.5, d2=4.5, h=2);
    translate([22.5,14.5,0]) cylinder(d1=2.5, d2=4.5, h=2);
}