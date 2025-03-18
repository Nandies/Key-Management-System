"""
API response models.
"""
from pydantic import BaseModel, Field
from typing import List, Dict, Optional, Any

class Key(BaseModel):
    """Model for a license key."""
    id: int = Field(..., description="Key ID in the database")
    value: str = Field(..., description="The actual key value")
    type: int = Field(..., description="Key type ID (0=Day, 1=Week, 2=Month, 3=Lifetime)")
    typeName: str = Field(..., description="Key type name")
    used: bool = Field(..., description="Whether the key is used")
    discordUsername: str = Field(..., description="Discord username of the user who used the key")

class KeysResponse(BaseModel):
    """Model for the response from the keys endpoint."""
    keys: List[Key] = Field(..., description="List of keys")

class KeyTypeStats(BaseModel):
    """Model for statistics about a specific key type."""
    total: int = Field(..., description="Total number of keys of this type")
    used: int = Field(..., description="Number of used keys of this type")
    available: int = Field(..., description="Number of available keys of this type")

class StatsResponse(BaseModel):
    """Model for the response from the stats endpoint."""
    totalKeys: int = Field(..., description="Total number of keys")
    usedKeys: int = Field(..., description="Number of used keys")
    availableKeys: int = Field(..., description="Number of available keys")
    keysByType: Dict[str, KeyTypeStats] = Field(..., description="Statistics by key type")

class SuccessResponse(BaseModel):
    """Model for a success response."""
    status: str = Field(..., description="Status of the operation")

class ErrorResponse(BaseModel):
    """Model for an error response."""
    error: str = Field(..., description="Error message")