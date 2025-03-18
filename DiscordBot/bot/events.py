"""
Event handlers for the Discord bot.
"""
import logging
import discord
import time
from collections import defaultdict
from discord.ext import commands
from utils.webhook import send_admin_notification

logger = logging.getLogger(__name__)

def register_events(bot: commands.Bot) -> None:
    """
    Register event handlers for the bot.
    
    Args:
        bot: The Discord bot instance
    """
    # Security tracking
    failed_attempts = defaultdict(int)
    cooldown_until = defaultdict(float)
    
    @bot.event
    async def on_ready():
        """Called when the bot is ready and connected to Discord."""
        logger.info(f"Logged in as {bot.user.name} (ID: {bot.user.id})")
        logger.info(f"Connected to {len(bot.guilds)} guilds")
        
        # Set bot status
        activity = discord.Activity(
            type=discord.ActivityType.listening,
            name=f"{bot.command_prefix}help"
        )
        await bot.change_presence(activity=activity)
        
        # Notify admins that the bot is online
        if bot.config.admin_webhook_url:
            await send_admin_notification(
                webhook_url=bot.config.admin_webhook_url,
                title="Bot Online",
                description=f"Key Management Bot is now online and connected to {len(bot.guilds)} servers.",
                color=0x00FF00  # Green
            )
    
    @bot.event
    async def on_command_error(ctx, error):
        """Global error handler for bot commands."""
        # Get user ID for security tracking
        user_id = ctx.author.id
        current_time = time.time()
        
        # Check if user is on cooldown
        if current_time < cooldown_until[user_id]:
            remaining_time = int(cooldown_until[user_id] - current_time)
            await ctx.send(f"Too many failed attempts. Please try again in {remaining_time} seconds.")
            return
            
        # Standard error handling
        if isinstance(error, commands.CommandNotFound):
            # Ignore command not found errors
            return
            
        if isinstance(error, commands.MissingRequiredArgument):
            await ctx.send(f"Missing required argument: {error.param.name}")
            return
            
        if isinstance(error, commands.BadArgument):
            await ctx.send(f"Invalid argument provided: {error}")
            return
            
        if isinstance(error, commands.MissingPermissions):
            # Track failed permission attempts
            failed_attempts[user_id] += 1
            
            # Check if we've exceeded the threshold
            if failed_attempts[user_id] >= bot.config.max_failed_attempts:
                # Set cooldown
                cooldown_until[user_id] = current_time + bot.config.cooldown_period
                failed_attempts[user_id] = 0
                
                # Log security event
                logger.warning(f"User {ctx.author} (ID: {user_id}) has been temporarily blocked due to too many permission failures")
                
                # Notify admins
                if bot.config.admin_webhook_url:
                    await send_admin_notification(
                        webhook_url=bot.config.admin_webhook_url,
                        title="Security Alert",
                        description=f"User {ctx.author} has been temporarily blocked due to too many permission failures",
                        color=0xFF0000  # Red
                    )
                
                await ctx.send("You don't have permission to use this command. Too many attempts - please try again later.")
                return
            
            await ctx.send("You don't have permission to use this command.")
            return
            
        if isinstance(error, commands.BotMissingPermissions):
            await ctx.send(f"I don't have the required permissions: {error.missing_permissions}")
            return
            
        # Log unexpected errors
        logger.error(f"Command error in {ctx.command}: {str(error)}")
        await ctx.send("An unexpected error occurred. The administrators have been notified.")
        
        # Notify admins about the error
        if bot.config.admin_webhook_url:
            await send_admin_notification(
                webhook_url=bot.config.admin_webhook_url,
                title="Command Error",
                description=f"Command: {ctx.command}\nUser: {ctx.author}\nError: {str(error)}",
                color=0xFF0000  # Red
            )
    
    @bot.event
    async def on_guild_join(guild):
        """Called when the bot joins a new server."""
        logger.info(f"Joined new guild: {guild.name} (ID: {guild.id})")
        
        # Notify admins about new server
        if bot.config.admin_webhook_url:
            await send_admin_notification(
                webhook_url=bot.config.admin_webhook_url,
                title="Joined New Server",
                description=f"Server: {guild.name}\nID: {guild.id}\nMembers: {guild.member_count}",
                color=0x00FF00  # Green
            )