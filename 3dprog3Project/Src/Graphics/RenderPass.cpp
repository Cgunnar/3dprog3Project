#include "pch.h"
#include "RenderPass.h"


ID3DBlob* LoadCSO(const std::string& filepath)
{
	std::ifstream file(filepath, std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error("Could not open CSO file");

	file.seekg(0, std::ios_base::end);
	size_t size = static_cast<size_t>(file.tellg());
	file.seekg(0, std::ios_base::beg);

	ID3DBlob* toReturn = nullptr;
	HRESULT hr = D3DCreateBlob(size, &toReturn);

	if (FAILED(hr))
		throw std::runtime_error("Could not create blob when loading CSO");

	file.read(static_cast<char*>(toReturn->GetBufferPointer()), size);
	file.close();

	return toReturn;
}