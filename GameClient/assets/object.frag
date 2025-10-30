#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 texCoord;

uniform sampler2D texture0;
uniform sampler2D texture1;

//uniform float mixVal;


//uniform vec3 viewPos;

void main() {
	FragColor = texture(texture0, texCoord);
	//FragColor = mix(texture(texture0, texCoord), texture(texture1, texCoord), mixVal);

	// ambient
	//vec3 ambient = light.ambient * material.ambient;

	// diffuse
	//vec3 norm = normalize(Normal);
	//vec3 lightDir = normalize(light.position - FragPos);
	//float diff = max(dot(norm, lightDir), 0.0);
	//vec3 diffuse = light.diffuse * (diff * material.diffuse);

	// specular
	//vec3 viewDir = normalize(viewPos - FragPos);
	//vec3 reflectDir = reflect(-lightDir, norm);
	//float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128); 
	// shininess is between 0 and 1
	//vec3 specular = light.specular * (spec * material.specular);

	//FragColor = vec4(ambient + diffuse + specular, 1.0);
}