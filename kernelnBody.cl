kernel void simulation(global float4* pos, global float4* vel,float timeScale)
{
	const float g = 6.67384e-11;
    //get workunit in the array
    unsigned int i = get_global_id(0);

	for(int j = 0; j < 256; j++) {
		float4 d = pos[i] - pos[j];

		float r = length(d);

		float force = (g * 10E04) / (r*r);
		vel[j] += d * force * timeScale / r;

		pos[j] += vel[i] * timeScale;
	}
}