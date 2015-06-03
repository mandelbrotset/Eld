void jacobi (half2 coords : WPOS,
			out half4 xNew : COLOR,
			uniform half alpha,
			uniform half rBeta,
			uniform samplerRECT x,
			uniform samplerRECT b)
{
	half4 xL = h4texRECT(x, coords - half2(1, 0));
	half4 xR = h4texRECT(x, coords + half2(1, 0));
	half4 xB = h4texRECT(x, coords - half2(0, 1));
	half4 xT = h4texRECT(x, coords + half2(0, 1));
}