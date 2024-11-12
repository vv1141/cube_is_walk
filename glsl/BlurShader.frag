#version 330 core

// Gaussian blur with linear sampling

in VsOut {
  vec2 uv;
} fsIn;

layout(location = 0) out vec4 colour;

uniform sampler2D textureSampler;
uniform bool horizontal;

uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main(){
  vec2 offset = 1.0 / textureSize(textureSampler, 0);
  vec3 result = texture(textureSampler, fsIn.uv).rgb * weight[0];
  if(horizontal){
    for(int i = 1; i < 5; ++i){
      result += texture(textureSampler, fsIn.uv + vec2(offset.x * i, 0.0)).rgb * weight[i];
      result += texture(textureSampler, fsIn.uv - vec2(offset.x * i, 0.0)).rgb * weight[i];
    }
  }
  else{
    for(int i = 1; i < 5; ++i){
      result += texture(textureSampler, fsIn.uv + vec2(0.0, offset.y * i)).rgb * weight[i];
      result += texture(textureSampler, fsIn.uv - vec2(0.0, offset.y * i)).rgb * weight[i];
    }
  }
  colour = vec4(result, 1.0);
}
