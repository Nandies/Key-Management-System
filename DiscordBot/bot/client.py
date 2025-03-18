"""
Discord bot client setup and initialization.
"""
import discord
from discord.ext import commands, tasks
import logging
import asyncio
from config import Config
from bot.commands import register_commands
from bot.events import register_events
from utils.cache import cleanup_expired_cache, vacuum_database

logger = logging.getLogger(__name__)

async def create_bot(config: Config) -> commands.Bot:
    """
    Create and configure the Discord bot.
    
    Args:
        config: Bot configuration
        
    Returns:
        commands.Bot: Configured bot instance
    """
    # Set up intents
    intents = discord.Intents.default()
    intents.message_content = True
    intents.members = True
    
    # Create bot instance
    bot = commands.Bot(
        command_prefix=config.bot_prefix,
        intents=intents,
        description="Key Management System Discord Bot",
        # Add help command auto-generation
        help_command=commands.DefaultHelpCommand(
            no_category="General Commands",
            dm_help=True  # Allow help command in DMs for privacy
        )
    )
    
    # Store config in bot for access in commands and events
    bot.config = config
    
    # Schedule periodic cache cleanup
    @tasks.loop(hours=24)
    async def cleanup_cache():
        logger.info("Running scheduled cache cleanup")
        try:
            # Remove expired entries
            await cleanup_expired_cache(expiry_minutes=config.cache_expiry_minutes * 10)  # 10x normal expiry
            # Optimize database
            await vacuum_database()
        except Exception as e:
            logger.error(f"Error during scheduled cache cleanup: {str(e)}")
    
    # Store the task in the bot for later reference
    bot.cleanup_cache_task = cleanup_cache
    
    # Register commands and events
    try:
        await register_commands(bot)
        register_events(bot)
    except Exception as e:
        logger.error(f"Error registering commands or events: {str(e)}")
        # Continue anyway - the bot can still function with default commands
    
    return bot