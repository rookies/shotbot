G = 2.54; /* 2.54mm grid */
T = 2; /* thickness */
$fn = 20;

module text1(txt) {
    rotate([0,0,90]) text(txt, size=4, halign="right", valign="center");
}
module text2(txt) {
    rotate([0,0,90]) text(txt, size=2, halign="right", valign="center");
}

linear_extrude(T) difference() {
    minkowski() {
        square([34*G,28*G], center=true);
        circle(G);
    }
    /* screw holes: */
    for (i = [1,-1]) {
        for (j = [1,-1]) {
            translate([i*15*G,j*12*G])
                circle(d=2.5);
        }
    }
    /* connector holes: */
    translate([-.5*G,10*G]) square([27*G,3.5], center=true);
}
!linear_extrude(T+1) {
    /* title text: */
    translate([0,-8*G]) rotate([0,0,180]) text("ShotBot v0.1", size=8, halign="center", valign="center");
    /* text 1: */
    translate([0,6*G]) {
        translate([-12*G,0]) text1("PWR IN");
        translate([-8*G,0]) text1("SERVO");
        translate([-3.5*G,0]) text1("PUMP");
        translate([.5*G,0]) text1("SOLENOID");
        translate([5.5*G,0]) text1("STEPPER");
        translate([11*G,0]) text1("ENDSTOP");
    }
    /* text 2: */
    translate([0,9*G]) {
        /* PWR IN */
        translate([-13*G,0]) text2("12V");
        translate([-12*G,0]) text2("5V");
        translate([-11*G,0]) text2("GND");
        /* SERVO */
        translate([-9*G,0]) text2("GND");
        translate([-8*G,0]) text2("5V");
        translate([-7*G,0]) text2("SIG");
        /* PUMP */
        translate([-4*G,0]) text2("GND");
        translate([-3*G,0]) text2("12V");
        /* SOLENOID */
        translate([0,0]) text2("GND");
        translate([G,0]) text2("12V");
        /* STEPPER */
        translate([3*G,0]) text2("ENA");
        translate([4*G,0]) text2("DIR");
        translate([5*G,0]) text2("PUL");
        translate([6*G,0]) text2("GND");
        translate([7*G,0]) text2("GND");
        translate([8*G,0]) text2("GND");
        /* ENDSTOP */
        translate([10*G,0]) text2("5V");
        translate([11*G,0]) text2("GND");
        translate([12*G,0]) text2("SIG");
    }
}