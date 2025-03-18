"""
Command registration for the Discord bot.
"""
from discord.ext import commands
import logging
from bot.commands.user_commands import setup as setup_user_commands
from bot.commands.admin_commands import setup as setup_admin_commands

logger = logging.getLogger(__name__)

async def register_commands(bot: commands.Bot) -> None:
    """
    Register all command modules with the bot.
    
    Args:
        bot: The Discord bot instance
    """
    logger.info("Registering bot commands")
    
    # Setup command modules
    await setup_user_commands(bot)
    await setup_admin_commands(bot)
    
    # Sync commands with Discord
    try:
        logger.info("Syncing application commands with Discord")
        synced_commands = await bot.tree.sync()
        logger.info(f"Successfully synced {len(synced_commands)} application commands")
    except Exception as e:
        logger.error(f"Failed to sync application commands: {e}")