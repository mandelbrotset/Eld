
precision mediump float;

varying float vAge;
varying vec3 vColor;

uniform sampler2D texture;

void main() {
     float alphaFactor;
     if(vAge <= 0.03) {
         alphaFactor = 0.03 * vAge;
     } else {
         alphaFactor = 0.03;
     }

     gl_FragColor = vec4(vColor, alphaFactor);
}