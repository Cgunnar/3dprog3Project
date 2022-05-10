cbuffer SomeIndex : register(b0, space0)
{
    int someIndex;
}


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    
}