void advect(float2 coords : WPOS,
			out float4 xNew : COLOR,
			uniform float timestep,
			uniform float rdx,
			uniform samplerRECT u,
			uniform samplerRECT x)
{
	float2 pos = coords - timestep * rdx * f2texRECT(u, coords);
	xNew = f4texRECTbilerp(x, pos);
}