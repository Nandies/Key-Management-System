"""
Discord webhook utilities for the Key Management System Discord Bot.
"""
import aiohttp
import asyncio
import logging
from datetime import datetime
from typing import Optional

logger = logging.getLogger(__name__)

# Rate limiting for webhooks
_last_webhook_time = 0
_webhook_cooldown = 2.0  # Seconds between webhook messages to avoid rate limits

async def send_webhook(
    webhook_url: str,
    title: str,
    description: str,
    color: int = 0x3498db,  # Blue
    **kwargs
) -> bool:
    """
    Send a message to a Discord webhook.
    
    Args:
        webhook_url: Discord webhook URL
        title: Message title
        description: Message description
        color: Embed color (hex color code as integer)
        **kwargs: Additional embed fields
        
    Returns:
        bool: True if successful, False otherwise
    """
    if not webhook_url:
        logger.warning("No webhook URL provided")
        return False
    
    # Create the embed
    embed = {
        "title": title,
        "description": description,
        "color": color,
        "timestamp": datetime.utcnow().isoformat()
    }
    
    # Add additional fields if provided
    for key, value in kwargs.items():
        embed[key] = value
    
    # Create the payload
    payload = {
        "embeds": [embed]
    }
    
    # Apply rate limiting to webhooks
    global _last_webhook_time
    current_time = asyncio.get_event_loop().time()
    time_since_last = current_time - _last_webhook_time
    
    if time_since_last < _webhook_cooldown:
        # Wait to avoid rate limiting
        await asyncio.sleep(_webhook_cooldown - time_since_last)
    
    try:
        async with aiohttp.ClientSession() as session:
            async with session.post(webhook_url, json=payload) as response:
                success = 200 <= response.status < 300
                
                if not success:
                    response_text = await response.text()
                    logger.error(f"Failed to send webhook (Status {response.status}): {response_text}")
                else:
                    # Update last webhook time only on success
                    _last_webhook_time = asyncio.get_event_loop().time()
                
                return success
    except Exception as e:
        logger.error(f"Error sending webhook: {str(e)}")
        return False

async def send_admin_notification(
    webhook_url: str,
    title: str,
    description: str,
    color: int = 0x3498db,  # Blue
    **kwargs
) -> bool:
    """
    Send an admin notification via Discord webhook.
    
    Args:
        webhook_url: Discord webhook URL
        title: Message title
        description: Message description
        color: Embed color (hex color code as integer)
        **kwargs: Additional embed fields
        
    Returns:
        bool: True if successful, False otherwise
    """
    return await send_webhook(
        webhook_url=webhook_url,
        title=f"[ADMIN] {title}",
        description=description,
        color=color,
        **kwargs
    )

async def send_notification(
    webhook_url: str,
    title: str,
    description: str,
    color: int = 0x3498db,  # Blue
    **kwargs
) -> bool:
    """
    Send a general notification via Discord webhook.
    
    Args:
        webhook_url: Discord webhook URL
        title: Message title
        description: Message description
        color: Embed color (hex color code as integer)
        **kwargs: Additional embed fields
        
    Returns:
        bool: True if successful, False otherwise
    """
    return await send_webhook(
        webhook_url=webhook_url,
        title=title,
        description=description,
        color=color,
        **kwargs
    )