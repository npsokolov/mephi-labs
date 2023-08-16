#include "byte_tools.h"
#include "piece.h"
#include <iostream>
#include <algorithm>


constexpr size_t BLOCK_SIZE = 1 << 14;


Piece::Piece(size_t index, size_t length, std::string hash) : 
            index_(index), 
            length_(length),
            hash_(hash)
{
    size_t len = length_;
    size_t offset = 0;
    while (len/BLOCK_SIZE>0){
        blocks_.emplace_back(Block{index,offset,BLOCK_SIZE,Block::Status::Missing,""});
        offset+=BLOCK_SIZE;
        len-=BLOCK_SIZE;
    }

    if (len>0){
        blocks_.emplace_back(Block{index,offset,len,Block::Status::Missing,""});
    }
}

bool Piece::HashMatches() const
{
    return (GetHash()==GetDataHash());
}

Block* Piece::FirstMissingBlock()
{
    for (size_t i = 0; i < blocks_.size(); i++)
    {
        if (blocks_[i].status==Block::Status::Missing){
            blocks_[i].status = Block::Status::Pending;
            return &blocks_[i];
        }
    }
    return nullptr;
}

size_t Piece::GetIndex() const
{
    return index_;
}

void Piece::SaveBlock(size_t blockOffset, std::string data)
{
    for (size_t i = 0; i < blocks_.size(); i++)
    {
        if (blocks_[i].offset==blockOffset){
            blocks_[i].data=data;
            blocks_[i].status=Block::Status::Retrieved;
            return;
        }
    }
    throw std::runtime_error("Wrong offset");
}

bool Piece::AllBlocksRetrieved() const
{
    for (size_t i = 0; i < blocks_.size(); i++)
    {
        if (blocks_[i].status!=Block::Status::Retrieved){
            return false;
        }
    }
    return true;
}

std::string Piece::GetData() const
{
    std::string data;
    for (size_t i = 0; i < blocks_.size(); i++)
    {
        data+=blocks_[i].data;
    }
    return data;
}

std::string Piece::GetDataHash() const
{
    std::string data;
    for (size_t i = 0; i < blocks_.size(); i++)
    {
        data+=blocks_[i].data;
    }
    return CalculateSHA1(data);
}

const std::string& Piece::GetHash() const
{
    return hash_;
}

void Piece::Reset()
{
    for (size_t i = 0; i < blocks_.size(); i++)
    {
        blocks_[i].data="";
        blocks_[i].status=Block::Status::Missing;
    }
}

