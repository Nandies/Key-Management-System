#!/usr/bin/env python3
"""
Main entry point for the Key Management System Discord Bot.
"""
import asyncio
from bot.client import create_bot
from utils.logger import setup_logger
from config import load_config

async def main():
    """Initialize and run the Discord bot."""
    # Load configuration
    config = load_config()
    
    # Setup logging
    logger = setup_logger()
    logger.info("Starting Key Management System Discord Bot")
    
    # Create and run the bot
    bot = await create_bot(config)
    
    try:
        # Start cache cleanup task when bot is ready
        @bot.event
        async def on_ready():
            # Import here to avoid circular imports
            from utils.cache import cleanup_expired_cache, vacuum_database
            
            # Do initial cache cleanup
            logger.info("Performing initial cache cleanup")
            await cleanup_expired_cache()
            await vacuum_database()
            
            # Start scheduled task if it exists and isn't running
            if hasattr(bot, 'cleanup_cache_task') and not bot.cleanup_cache_task.is_running():
                bot.cleanup_cache_task.start()
        
        logger.info("Bot is now running. Press Ctrl+C to exit.")
        await bot.start(config.discord_token)
    except KeyboardInterrupt:
        logger.info("Received keyboard interrupt. Shutting down...")
        # Cancel cleanup task if it's running
        if hasattr(bot, 'cleanup_cache_task') and bot.cleanup_cache_task.is_running():
            bot.cleanup_cache_task.cancel()
        await bot.close()
    except Exception as e:
        logger.error(f"Unexpected error occurred: {str(e)}")
        # Cancel cleanup task if it's running
        if hasattr(bot, 'cleanup_cache_task') and bot.cleanup_cache_task.is_running():
            bot.cleanup_cache_task.cancel()
        await bot.close()

if __name__ == "__main__":
    asyncio.run(main())