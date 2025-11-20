// ============================================================================
// POST-PROCESSING SHADER
// ============================================================================
// Effets de post-processing HD-2D style:
// - Tilt-shift blur (zone nette autour joueur, flou progressif)
// - Ambient occlusion diffus (assombrissement doux des bords)
// - Bloom (glow sur zones lumineuses)
// - Saturation (vibrance des couleurs)
// ============================================================================
#version 120
uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 resolution;
// Paramètres actifs
uniform float vignetteStrength; // Force de l'ambient occlusion
uniform float glowIntensity; // Intensité du bloom
uniform float saturation; // Saturation des couleurs
uniform float ambientOcclusion; // Intensité du tilt-shift blur
// ============================================================================
// TEXTURE SAMPLING
// ============================================================================
vec3 sampleTex(vec2 uv) {
return texture2D(tex, clamp(uv, 0.0, 1.0)).rgb;
}
// ============================================================================
// TILT-SHIFT BLUR
// ============================================================================
vec3 applyTiltShift(vec2 uv, float intensity) {
if (intensity <= 0.0) return sampleTex(uv);



vec2 pixelSize = 1.0 / resolution;

float distFromCenter = abs(uv.y - 0.5) * 2.0;



// Zone nette au centre (autour du joueur)

float blurAmount = smoothstep(0.15, 0.7, distFromCenter) * intensity;



if (blurAmount < 0.01) return sampleTex(uv);



vec3 color = vec3(0.0);

float totalWeight = 0.0;



// Blur gaussien avec rayon variable

float radius = blurAmount * 8.0;

int samples = int(radius) + 1;

if (samples > 10) samples = 10; // Limite pour performance



for (int x = -samples; x <= samples; x++) {

    for (int y = -samples; y <= samples; y++) {

        vec2 offset = vec2(float(x), float(y)) * pixelSize;

        float dist = length(vec2(x, y));

        float weight = exp(-dist * dist / (2.0 * radius));



        color += sampleTex(uv + offset) * weight;

        totalWeight += weight;

    }

}



return color / totalWeight;
}
// ============================================================================
// BLOOM
// ============================================================================
vec3 applyBloom(vec2 uv, vec3 baseColor, float intensity) {
if (intensity <= 0.0) return baseColor;



float brightness = dot(baseColor, vec3(0.299, 0.587, 0.114));



// Bloom seulement sur zones lumineuses

if (brightness < 0.6) return baseColor;



vec2 pixelSize = 1.0 / resolution;

vec3 bloom = vec3(0.0);

float totalWeight = 0.0;



// Échantillonnage 5x5

for (float x = -2.0; x <= 2.0; x += 1.0) {

    for (float y = -2.0; y <= 2.0; y += 1.0) {

        vec2 offset = vec2(x, y) * pixelSize * 2.0;

        float dist = length(vec2(x, y));

        float weight = exp(-dist * dist / 4.0);



        bloom += sampleTex(uv + offset) * weight;

        totalWeight += weight;

    }

}



bloom /= totalWeight;

float bloomAmount = (brightness - 0.6) * intensity * 2.0;



return baseColor + bloom * bloomAmount;
}
// ============================================================================
// AMBIENT OCCLUSION (SSAO simplifié basé sur les bords des formes)
// ============================================================================
float applyAO(vec2 uv, float strength) {
if (strength <= 0.0) return 1.0;

vec2 pixelSize = 1.0 / resolution;
vec3 centerColor = sampleTex(uv);
float centerLuminance = dot(centerColor, vec3(0.299, 0.587, 0.114));

float occlusion = 0.0;
int aoSamples = 0;

// Échantillonnage en cercle autour du pixel
float aoRadius = 3.0;
for (float angle = 0.0; angle < 6.28318; angle += 0.785398) { // 8 directions
    for (float dist = 1.0; dist <= aoRadius; dist += 1.0) {
        vec2 offset = vec2(cos(angle), sin(angle)) * dist * pixelSize;
        vec3 sampleColor = sampleTex(uv + offset);
        float sampleLuminance = dot(sampleColor, vec3(0.299, 0.587, 0.114));

        // Si le sample est significativement plus sombre, il y a occlusion
        float diff = centerLuminance - sampleLuminance;
        if (diff > 0.1) {
            // Plus le sample est proche et sombre, plus l'occlusion est forte
            float weight = (1.0 - dist / aoRadius) * diff;
            occlusion += weight;
        }
        aoSamples++;
    }
}

// Normaliser et appliquer
occlusion = occlusion / float(aoSamples) * strength;
return 1.0 - clamp(occlusion, 0.0, 0.6);
}
// ============================================================================
// SATURATION
// ============================================================================
vec3 applySaturation(vec3 color, float sat) {
float gray = dot(color, vec3(0.299, 0.587, 0.114));

return mix(vec3(gray), color, sat);
}
// ============================================================================
// MAIN
// ============================================================================
void main()
{
// Normaliser les UV et inverser Y

vec2 uv = gl_TexCoord[0].xy / texSize;

uv.y = 1.0 - uv.y;



// Tilt-shift blur

vec3 color = applyTiltShift(uv, ambientOcclusion);



// Bloom

color = applyBloom(uv, color, glowIntensity);



// Saturation

color = applySaturation(color, saturation);



// Ambient occlusion

color *= applyAO(uv, vignetteStrength);



// Clamp final

color = clamp(color, 0.0, 1.0);



gl_FragColor = vec4(color, 1.0);
}
