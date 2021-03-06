uniform sampler2D uTexUnit;

varying vec2 vTexCoord;
varying vec3 vColor;
varying vec3 vNormal;
varying vec3 vPos;
varying vec4 vEye;
varying vec3 vLightVector;

//the normals are interpolated across the face and shading is computed per pixel

void main() {
  if(vPos.y < -1.7){ discard; }
  vec3 uLightCol = vec3(1.0, 1.0, 1.0); //set to white light
  float shininess = 0.0; //alpha

  vec4 texColor0 = vec4(vColor.x, vColor.y, vColor.z, 1.0);
  vec4 texColor1 = texture2D(uTexUnit, vTexCoord);
  if(texColor1[2] > .7){ //white
    shininess += 0.0;
  }
  else if(texColor1[2] > .5){
    shininess += 3.0;
    //discard;
  }
  else if(texColor1[0] > .2){ // set imperial blue
    texColor1[0] = 0.25;
    texColor1[1] = 0.25;
    texColor1[2] = 0.45;
    shininess += 3.0;
    //discard;
  }
  else { //not texture
    texColor1 = vec4(vColor.x, vColor.y, vColor.z, 1.0);
    shininess += 4.0;
  }

//first compute diffuse

  vec3 L = normalize(vLightVector - vPos);
  vec3 newNorm = normalize(vNormal);

  float dotProd = dot(newNorm, L);
  if(dotProd > 0.7){
    dotProd = 0.8;
  }
  else if(dotProd > 0.4){
    dotProd = 0.5;
  }
  else{
    dotProd = 0.2;
  }
  dotProd = max(dotProd, 0.0);
  float dotProd_r = uLightCol.x*dotProd;
  float dotProd_g = uLightCol.y*dotProd;
  float dotProd_b = uLightCol.z*dotProd;
  //diffuse and ambient
  vec3 newColor = vec3(texColor1.r*dotProd_r + texColor1.r*.1, texColor1.g*dotProd_g + texColor1.g*.1, texColor1.b*dotProd_b + texColor1.b*.1);

  //now compute specular
  float R_x = 2.0*dotProd*newNorm.x - L.x;
  float R_y = 2.0*dotProd*newNorm.y - L.y;
  float R_z = 2.0*dotProd*newNorm.z - L.z;
  vec3 R = reflect(-L, newNorm);

  vec3 vEyePos = normalize(vEye.xyz);
  float spec = pow(max(dot(R, vEyePos), 0.0), shininess);

  float spec_r = uLightCol.x*spec;
  float spec_g = uLightCol.y*spec;
  float spec_b = uLightCol.z*spec;
  //specular
  if(dotProd > 0.0){
    newColor += vec3(texColor1.r*spec_r, texColor1.g*spec_g, texColor1.b*spec_b);
  }
  //full phong
  gl_FragColor = vec4(newColor.r, newColor.g, newColor.b, 1.0);
  //gl_FragColor += vec4(abs(normalize(vNormal)), 1.0); // Show Normals
}
