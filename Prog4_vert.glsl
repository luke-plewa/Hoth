uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;

uniform vec3 uLight;
uniform vec4 uView;

attribute vec3 aPosition;
attribute vec3 aNormal;

attribute vec2 aTexCoord;
uniform vec3 aColor;

varying vec3 vColor;
varying vec3 vNormal;
varying vec3 vPos;
varying vec2 vTexCoord;
varying vec4 vEye;
varying vec3 vLightVector;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
  vec4 vPosition;
  vTexCoord = aTexCoord;

  vec4 Normal = vec4(aNormal, 0.0);
  vNormal = (uNormalMatrix * Normal).xyz;
  vPosition = uModelMatrix* vec4(aPosition.xyz, 1.0);
  gl_Position = uProjMatrix * uViewMatrix * vPosition;

  vPos = vec3(vPosition.x, vPosition.y, vPosition.z);
  vColor = vec3(aColor.r+0.2, aColor.g+0.2, aColor.b+0.2);
  vLightVector = uLight;
  vEye = uView;

  //pass to fragment shader
}
