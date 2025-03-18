"""
API client for the Key Management System.
"""
import aiohttp
import logging
import json
from typing import Dict, Any, Optional, Union

logger = logging.getLogger(__name__)

class APIError(Exception):
    """Custom exception for API errors."""
    def __init__(self, status: int, message: str):
        self.status = status
        self.message = message
        super().__init__(f"API Error ({status}): {message}")

class KeyManagementAPI:
    """Client for the Key Management System API."""
    
    def __init__(self, base_url: str, api_key: str):
        """
        Initialize the API client.
        
        Args:
            base_url: Base URL for the API
            api_key: API key for authentication
        """
        self.base_url = base_url.rstrip('/')
        self.api_key = api_key
        self.headers = {
            "X-API-Key": api_key,
            "Content-Type": "application/json"
        }
    
    async def _request(
        self,
        method: str,
        endpoint: str,
        data: Optional[Dict[str, Any]] = None,
        timeout: float = 10.0,
        retries: int = 3
    ) -> Dict[str, Any]:
        """
        Send a request to the API.
        
        Args:
            method: HTTP method (GET, POST, PUT, DELETE)
            endpoint: API endpoint
            data: Request data
            
        Returns:
            Dict[str, Any]: Response data
            
        Raises:
            APIError: If the API returns an error
        """
        url = f"{self.base_url}/{endpoint.lstrip('/')}"
        
        retry_count = 0
        last_error = None
        
        while retry_count < retries:
            try:
                async with aiohttp.ClientSession() as session:
                    kwargs = {
                        "headers": self.headers,
                        "timeout": aiohttp.ClientTimeout(total=timeout)
                    }
                    
                    if data is not None:
                        kwargs["json"] = data
                    
                    async with session.request(method, url, **kwargs) as response:
                        response_text = await response.text()
                        
                        try:
                            response_data = json.loads(response_text)
                        except json.JSONDecodeError:
                            # Not a JSON response
                            if response.status >= 400:
                                raise APIError(response.status, response_text)
                            return {"status": "success", "message": response_text}
                        
                        if response.status >= 400:
                            error_message = response_data.get("error", "Unknown error")
                            raise APIError(response.status, error_message)
                        
                        return response_data
            except (aiohttp.ClientError, asyncio.TimeoutError) as e:
                # Network or connection error
                last_error = e
                retry_count += 1
                
                if retry_count < retries:
                    # Exponential backoff: 1s, 2s, 4s, etc.
                    wait_time = 2 ** (retry_count - 1)
                    logger.warning(f"API request failed, retrying in {wait_time}s... ({retry_count}/{retries})")
                    await asyncio.sleep(wait_time)
                else:
                    logger.error(f"Connection error after {retries} retries: {str(e)}")
                    raise APIError(0, f"Connection error: {str(e)}")
        
        # This should never happen, but just in case
        raise APIError(0, f"Connection error: {str(last_error)}")
    
    async def get_all_keys(self) -> Dict[str, Any]:
        """
        Get all keys from the API.
        
        Returns:
            Dict[str, Any]: Response containing all keys
        """
        return await self._request("GET", "keys")
    
    async def get_keys_by_type(self, key_type: Union[str, int]) -> Dict[str, Any]:
        """
        Get keys of a specific type.
        
        Args:
            key_type: Key type ID (0=Day, 1=Week, 2=Month, 3=Lifetime)
            
        Returns:
            Dict[str, Any]: Response containing keys of the specified type
        """
        return await self._request("GET", f"keys/type/{key_type}")
    
    async def add_key(self, key_value: str, key_type: Union[str, int]) -> bool:
        """
        Add a new key.
        
        Args:
            key_value: The key value
            key_type: Key type ID (0=Day, 1=Week, 2=Month, 3=Lifetime)
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            data = {
                "value": key_value,
                "type": int(key_type)
            }
            
            response = await self._request("POST", "keys", data)
            return response.get("status") == "success"
        except APIError:
            return False
        except Exception as e:
            logger.error(f"Error adding key: {str(e)}")
            return False
    
    async def mark_key_as_used(self, key_id: Union[str, int], discord_username: str) -> bool:
        """
        Mark a key as used.
        
        Args:
            key_id: The key ID
            discord_username: Discord username of the user
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            data = {
                "discordUsername": discord_username
            }
            
            response = await self._request("PUT", f"keys/{key_id}/use", data)
            return response.get("status") == "success"
        except APIError:
            return False
        except Exception as e:
            logger.error(f"Error marking key as used: {str(e)}")
            return False
    
    async def mark_key_as_unused(self, key_id: Union[str, int]) -> bool:
        """
        Mark a key as unused.
        
        Args:
            key_id: The key ID
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            response = await self._request("PUT", f"keys/{key_id}/unuse")
            return response.get("status") == "success"
        except APIError:
            return False
        except Exception as e:
            logger.error(f"Error marking key as unused: {str(e)}")
            return False
    
    async def get_stats(self) -> Dict[str, Any]:
        """
        Get key statistics.
        
        Returns:
            Dict[str, Any]: Key statistics
        """
        return await self._request("GET", "stats")