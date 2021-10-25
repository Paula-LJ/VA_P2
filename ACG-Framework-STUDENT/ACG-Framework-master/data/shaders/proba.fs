#define PI 3.14159265359
#define RECIPROCAL_PI 0.3183098861837697

const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

// Variables necessàries per calcular el IBL
uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

// Variables afegides
uniform vec3 lightpos;
uniform vec3 u_camera_pos;
uniform mat4 u_model;
uniform mat4 u_viewprojection;

uniform vec4 u_color;
uniform samplerCube u_texture;
uniform sampler2D u_albedo_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_metalness_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_LUT_BRDF;
//uniform sampler2D u_opacity_texture;
uniform sampler2D u_ao_texture;
//uniform sampler2D u_emissive_texture;

uniform float u_time;

varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec3 u_camera_position;
uniform vec3 u_light_position;
uniform vec3 u_light_specular;

uniform float u_rough_val;
uniform float u_metal_val;
//uniform float u_has_opacity;
uniform float u_has_ao;
//uniform float u_has_emissive;
uniform float u_has_pbr ;
uniform float u_has_ibl;



struct PBRMat
{
	float metalness;
	float roughness;
	float ao; //cal almenys una propietat d'aqueestes per fer el rendering a getPBR
	//float opacity;
	//float emissive;
	float alpha;
	vec3 specularColor;
	vec3 diffColor;

	
};

// degamma
vec3 gamma_to_linear(vec3 color)
{
	return pow(color, vec3(GAMMA));
}

// gamma
vec3 linear_to_gamma(vec3 color)
{
	return pow(color, vec3(INV_GAMMA));
}

vec3 getReflectionColor(vec3 r, float roughness)
{
	float lod = roughness * 5.0;

	vec4 color;

	if(lod < 1.0) color = mix( textureCube(u_texture, r), textureCube(u_texture_prem_0, r), lod );
	else if(lod < 2.0) color = mix( textureCube(u_texture_prem_0, r), textureCube(u_texture_prem_1, r), lod - 1.0 );
	else if(lod < 3.0) color = mix( textureCube(u_texture_prem_1, r), textureCube(u_texture_prem_2, r), lod - 2.0 );
	else if(lod < 4.0) color = mix( textureCube(u_texture_prem_2, r), textureCube(u_texture_prem_3, r), lod - 3.0 );
	else if(lod < 5.0) color = mix( textureCube(u_texture_prem_3, r), textureCube(u_texture_prem_4, r), lod - 4.0 );
	else color = textureCube(u_texture_prem_4, r);

	//color.rgb = linear_to_gamma(color.rgb);

	return color.rgb;
}

//Javi Agenjo Snipet for Bump Mapping
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv){
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );

	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}

vec3 perturbNormal( vec3 N, vec3 V, vec2 texcoord, vec3 normal_pixel ){
	#ifdef USE_POINTS
	return N;
	#endif

	// assume N, the interpolated vertex normal and
	// V, the view vector (vertex to eye)
	//vec3 normal_pixel = texture2D(normalmap, texcoord ).xyz;
	normal_pixel = normal_pixel * 255./127. - 128./127.;
	mat3 TBN = cotangent_frame(N, V, texcoord);
	return normalize(TBN * normal_pixel);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

 
//Convert 0-Inf range to 0-1 range so we can
//display info on screen

vec3 toneMap(vec3 color)
{
    return color / (color + vec3(1.0));
}

// Uncharted 2 tone map
// see: http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 toneMapUncharted2Impl(vec3 color)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec3 toneMapUncharted(vec3 color)
{
    const float W = 11.2;
    color = toneMapUncharted2Impl(color * 2.0);
    vec3 whiteScale = 1.0 / toneMapUncharted2Impl(vec3(W));
    return color * whiteScale;
}

void getMaterialProperties(PBRMat mat , vec2 texcoord){
	
	mat.metalness = texture2D(u_metalness_texture, texcoord ).x * u_metal_val;
	mat.roughness = texture2D(u_roughness_texture, texcoord ).x * u_rough_val;
	
	// Passem a linial la textura albedo (color difús del material)
	mat.diffColor = gamma_to_linear(texture2D(u_albedo_texture, texcoord ).xyz);
	mat.specularColor= vec3(0.04)*(1-mat.metalness); // Volem el 4% de material dielèctric, que com és el contrari a la metalness i per això (1-metalness)
	
	//if(u_has_opacity != 0  ){
		//mat.opacity = texture2D(u_opacity_texture,texcoord).x;
	//}
	//else {
		//mat.opacity = 1.0;
	//}

	if(u_has_ao != 0  ){
		mat.ao = texture2D(u_ao_texture,texcoord).x;
	}
	else {
		mat.ao = 1.0;
	}

	//if(u_has_emissive != 0  ){
		//material.emissive = gamma_to_linear(texture2D(u_emissive_texture, texcoord ).xyz);
	//}
	//else {
		//material.emissive = vec3(0.0);
	//}
}

vec3 getPBR(PBRMat mat)
{
	//Càlculs
	vec3 L = lightpos - v_world_position; 
	L = normalize(L);

	vec3 V = u_camera_pos-v_world_position;
	V = normalize(V);

	vec3 N = perturbNormal(normalize(v_normal), V, v_uv, texture2D(u_normal_texture, v_uv).xyz);
	vec3 H = normalize(L+V); 

	vec3 NdotV = clamp(dot(N,V), 0.0, 1.0);
	vec3 NdotL = clamp(dot(N,L), 0.0, 1.0);
	vec3 NdotH = clamp(dot(N,H), 0.0, 1.0);
	vec3 LdotH = clamp(dot(L,H), 0.0, 1.0);
	vec3 VdotH = clamp(dot(V,H), 0.0, 1.0);

	vec3 R = -normalize(reflect(L, N));

	vec3 BRDF;
	vec3 IBL;

	//if per calcular la BRDF (direct lighting).
	//if (u_has_pbr != 0){
		//f_diffuse
		////vec3 c_diff = mat.baseColor * (1-mat.metalness);
		//vec3 c_diff = mix(vec3(0.0), mat.diffColor, 1); //discord
		//vec3 f_diffuse = c_diff/PI;
	
		//f_specular
	vec3 F_0 = mat.specularColor;
		//mat.alpha = pow(mat.roughness,2);

		//vec3 F = F_0 + (1-F_0)*(1-NdotL)**5; 
		//vec3 G = min(vec3(1), (2*(NdotH)*(NdotV))/VdotH, (2*(NdotH)*(NdotL))/VdotH);
		//vec3 D = (mat.alpha**2)/(PI((NdotH)**2(mat.alpha**2 -1)+1)**2);

		//vec3 f_specular = (F*G*D) / (4*(NdotL)*(NdotV));
	
		//BRDF
		//BRDF = f_diffuse + f_specular;
	//}
	//else {
		//BRDF = vec3(0.0); //Posem aquest valor de manera aleatòria per tal que tingui algun valor en cas de no tenir PBR.
	//}

	//if per calcular la IBL (Indirect Lighting).
	if(u_has_ibl != 0){

		//IBL specular
		vec3 specularSample = getReflectionColor(R, mat.roughness);
		vec2 LUT_coord = (NdotL, mat.roughness); //coordenades d'una taula LUT

		vec3 brdf2D = texture2D(u_LUT_BRDF,LUT_coord); //agafem valors de la taula LUT predeterminada
		float cos_theta = max(NdotV,0.0); //Fem aquest producte perquè és una de les dues coordenades del LUT (quadradet vermell i verd)
		vec3 F_s = FresnelSchlickRoughness(cos_theta, F_0, mat.roughness);

		vec3 SpecularBRDF =  F_s * vec3(brdf2D.x) + vec3(brdf2D.y);
		vec3 SpecularIBL = specularSample * SpecularBRDF;
		
		//IBL difusse
		vec3 diffuseSample = getReflectionColor(N, mat.roughness); 
		vec3 diffuseColor = mat.diffColor;

		vec3 DiffuseIBL = diffuseSample * diffuseColor;
		
		DiffuseIBL *= (1-F_s); //Per evitar que es trenqui el principi de conservació de l'energia.

		//IBL final
		IBL = DiffuseIBL + SpecularIBL;
	}
	else {
		IBL = vec3(0.0); //Posem aquest valor de manera aleatòria per tal que tingui algun valor en cas de no tenir PBR.
	}
	
	// Unió de les llums
	//vec3 direct_color = vec3(BRDF * NdotL); //* u_light_specular); //fem el producte de la formula f*Li*(n*l) on Li és la radiança incident, però no la posem perquè tenim en compte només una llum.
	vec3 indirect_color = vec3(IBL*mat.ao); 

	vec3 color_rgb =  indirect_color; //direct_color +
	
	//Apliquem el tone mapping --> HDRE
	//color_rgb.xyz = (toneMap(color_rgb.xyz)); 
	
	return color_rgb;
}


void main()
{
	
	// 1. Create Material
	PBRMat mat= PBRMat(0.0, 0.0, 0.0, 0.0, vec3(0), vec3(0));
	
	// 2. Fill Material
	getMaterialProperties(mat, v_uv);

	// 3. Shade (Direct + Indirect); 
	// 4. Apply Tonemapping; 
	vec3 color = getPBR(mat);

	//5. Any extra texture to apply after tonemapping; 
	//vec4 color_textplus = vec4(color, textura); // FALTA AFEGIR UNBA TEXTURA EXTRA!!! E IRÍA DESPUÉS DEL LINEAR TO GAMMA
	
	//Last step: to gamma space
	vec4 color_gamma =  (linear_to_gamma(color), 1.0);
	
	gl_FragColor = color_gamma;
}