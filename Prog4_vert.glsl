uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;

uniform vec3 uLight;
uniform vec4 uView;

uniform vec3 options; // laserProgress, myRot, manipulate
uniform vec3 uLaser;

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

  vec4 Normal = vec4(aNormal, 0.0f);
  vNormal = (uNormalMatrix * Normal).xyz;

  if(options.z == 1.0f){ // if manipulate
    vec3 hit = (aPosition.xyz - uLaser) * -1.0f;
    vec3 tempPos = aPosition.xyz + aNormal*0.01f*options.y*rand(vec2(hit.x, hit.y));
    tempPos.y += 0.01f*options.y;

    float Angle = options.y * 0.01f;
    mat4 tempRot = mat4( cos( Angle ), -sin( Angle ), 0.0, 0.0,
                        sin( Angle ),  cos( Angle ), 0.0, 0.0,
                         0.0,           0.0, 1.0, 0.0,
                         0.0,           0.0, 0.0, 1.0 );
    vNormal =  (uNormalMatrix *tempRot* Normal).xyz;
    vPosition = uModelMatrix* tempRot*vec4(tempPos, 1.0f);
  }
  else{
    vPosition = uModelMatrix* vec4(aPosition.xyz, 1.0f);
  }
  gl_Position = uProjMatrix * uViewMatrix * vPosition;

  vPos = vec3(vPosition.x, vPosition.y, vPosition.z);
  vColor = aColor;
  vLightVector = uLight;
  vEye = uView;

  //pass to fragment shader
}
