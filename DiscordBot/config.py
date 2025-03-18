"""
Configuration management for the Key Management System Discord Bot.
"""
import os
from pathlib import Path
from typing import Optional
from dataclasses import dataclass
from dotenv import load_dotenv

@dataclass
class Config:
    """Configuration container for the bot."""
    # Discord settings
    discord_token: str
    bot_prefix: str
    
    # API settings
    api_base_url: str
    api_key: str
    
    # Logging
    log_level: str
    
    # Webhook URLs
    admin_webhook_url: Optional[str]
    notification_webhook_url: Optional[str]
    
    # Cache settings
    cache_enabled: bool
    cache_expiry_minutes: int
    
    # Security settings
    max_failed_attempts: int = 5  # Max number of failed login attempts before temporary ban
    cooldown_period: int = 300    # Cooldown period in seconds after max failed attempts

def load_config() -> Config:
    """
    Load configuration from environment variables.
    
    Returns:
        Config: Configuration object with all settings
    """
    # Find and load .env file
    env_path = Path('.') / '.env'
    if env_path.exists():
        load_dotenv(dotenv_path=env_path)
    else:
        load_dotenv()  # Try to load from environment
    
    # Get required values with validation
    discord_token = os.getenv('DISCORD_TOKEN')
    if not discord_token:
        raise ValueError("DISCORD_TOKEN must be provided in .env file or environment")
    
    api_base_url = os.getenv('API_BASE_URL')
    if not api_base_url:
        raise ValueError("API_BASE_URL must be provided in .env file or environment")
    
    api_key = os.getenv('API_KEY')
    if not api_key:
        raise ValueError("API_KEY must be provided in .env file or environment")
    
    # Get optional values with defaults
    bot_prefix = os.getenv('BOT_PREFIX', '!')
    log_level = os.getenv('LOG_LEVEL', 'INFO')
    admin_webhook_url = os.getenv('ADMIN_WEBHOOK_URL')
    notification_webhook_url = os.getenv('NOTIFICATION_WEBHOOK_URL')
    
    # Parse boolean and integer values
    cache_enabled = os.getenv('CACHE_ENABLED', 'true').lower() in ('true', 'yes', '1', 't', 'y')
    
    try:
        cache_expiry_minutes = int(os.getenv('CACHE_EXPIRY_MINUTES', '5'))
    except ValueError:
        cache_expiry_minutes = 5  # Default to 5 minutes if invalid value
        
    # Parse security settings
    try:
        max_failed_attempts = int(os.getenv('MAX_FAILED_ATTEMPTS', '5'))
    except ValueError:
        max_failed_attempts = 5
        
    try:
        cooldown_period = int(os.getenv('COOLDOWN_PERIOD', '300'))
    except ValueError:
        cooldown_period = 300
    
    return Config(
        discord_token=discord_token,
        bot_prefix=bot_prefix,
        api_base_url=api_base_url,
        api_key=api_key,
        log_level=log_level,
        admin_webhook_url=admin_webhook_url,
        notification_webhook_url=notification_webhook_url,
        cache_enabled=cache_enabled,
        cache_expiry_minutes=cache_expiry_minutes,
        max_failed_attempts=max_failed_attempts,
        cooldown_period=cooldown_period
    )