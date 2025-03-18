"""
User-facing commands for the Discord bot.
"""
import discord
from discord import app_commands
from discord.ext import commands
import logging
from api.client import KeyManagementAPI
from utils.webhook import send_notification
from utils.cache import get_cache, update_cache

logger = logging.getLogger(__name__)

async def setup(bot: commands.Bot) -> None:
    """
    Set up user commands.
    
    Args:
        bot: The Discord bot instance
    """
    # Create API client
    api = KeyManagementAPI(bot.config.api_base_url, bot.config.api_key)
    
    # Add commands to the bot
    
    # User commands cooldown cache
    user_cooldowns = {}
    
    # Helper function to check and apply cooldowns
    async def check_cooldown(user_id: int, command: str, cooldown_seconds: int = 30) -> bool:
        """
        Check if a user is on cooldown for a command.
        
        Args:
            user_id: Discord user ID
            command: Command name
            cooldown_seconds: Cooldown period in seconds
            
        Returns:
            bool: True if user can use command, False if on cooldown
        """
        key = f"{user_id}:{command}"
        current_time = discord.utils.utcnow().timestamp()
        
        if key in user_cooldowns:
            last_used = user_cooldowns[key]
            if current_time - last_used < cooldown_seconds:
                # User is on cooldown
                return False
        
        # Update cooldown timestamp
        user_cooldowns[key] = current_time
        return True
    
    @bot.tree.command(
        name="getkey",
        description="Get a license key for the specified subscription type"
    )
    @app_commands.describe(
        key_type="Type of subscription key to request"
    )
    @app_commands.choices(key_type=[
        app_commands.Choice(name="Daily", value="0"),
        app_commands.Choice(name="Weekly", value="1"),
        app_commands.Choice(name="Monthly", value="2"),
        app_commands.Choice(name="Lifetime", value="3")
    ])
    async def getkey(interaction: discord.Interaction, key_type: app_commands.Choice[str]):
        """
        Get a license key for the requested subscription type.
        
        Args:
            interaction: The Discord interaction
            key_type: Type of key to request (Daily, Weekly, Monthly, Lifetime)
        """
        # Apply rate limiting (1 key per minute)
        if not await check_cooldown(interaction.user.id, "getkey", 60):
            remaining_time = int(60 - (discord.utils.utcnow().timestamp() - user_cooldowns[f"{interaction.user.id}:getkey"]))
            await interaction.response.send_message(
                f"You're requesting keys too quickly. Please wait {remaining_time} seconds before trying again.",
                ephemeral=True
            )
            return
            
        # Defer response as this might take some time
        await interaction.response.defer(ephemeral=True)
        
        # Get user info
        user_id = interaction.user.id
        username = f"{interaction.user.name}"
        
        # Check if keys are available from cache first
        cache_key = f"keys_type_{key_type.value}"
        cached_data = await get_cache(cache_key, bot.config.cache_enabled, bot.config.cache_expiry_minutes)
        
        if cached_data and cached_data.get("available", 0) == 0:
            await interaction.followup.send(
                f"Sorry, there are no {key_type.name} keys available at the moment. Please contact an administrator.",
                ephemeral=True
            )
            return
        
        try:
            # Get available keys by type
            keys_response = await api.get_keys_by_type(key_type.value)
            
            # Filter unused keys
            unused_keys = [k for k in keys_response.get("keys", []) if not k.get("used")]
            
            # Update cache with current availability
            if bot.config.cache_enabled:
                await update_cache(
                    cache_key,
                    {
                        "available": len(unused_keys),
                        "total": len(keys_response.get("keys", []))
                    }
                )
            
            if not unused_keys:
                await interaction.followup.send(
                    f"Sorry, there are no {key_type.name} keys available at the moment. Please contact an administrator.",
                    ephemeral=True
                )
                return
            
            # Get the first available key
            key = unused_keys[0]
            key_id = key.get("id")
            key_value = key.get("value")
            
            # Mark the key as used
            success = await api.mark_key_as_used(key_id, username)
            
            if success:
                # Send the key to the user
                embed = discord.Embed(
                    title=f"Your {key_type.name} Key",
                    description=f"Here is your requested license key:\n`{key_value}`",
                    color=discord.Color.green()
                )
                embed.add_field(name="Type", value=key_type.name, inline=True)
                embed.add_field(name="User", value=username, inline=True)
                embed.set_footer(text="Keep this key private and do not share it!")
                
                await interaction.followup.send(embed=embed, ephemeral=True)
                
                # Log key assignment
                logger.info(f"Key {key_value} ({key_type.name}) assigned to {username}")
                
                # Send notification via webhook if configured
                if bot.config.notification_webhook_url:
                    await send_notification(
                        webhook_url=bot.config.notification_webhook_url,
                        title="Key Assigned",
                        description=f"A {key_type.name} key was assigned to {username}",
                        color=0x3498db  # Blue
                    )
            else:
                await interaction.followup.send(
                    "There was an error assigning your key. Please try again later or contact an administrator.",
                    ephemeral=True
                )
        except Exception as e:
            logger.error(f"Error in getkey command: {str(e)}")
            await interaction.followup.send(
                "There was an error processing your request. Please try again later.",
                ephemeral=True
            )
    
    @bot.tree.command(
        name="checkkey",
        description="Check the status of a license key"
    )
    @app_commands.describe(
        key="The license key to check"
    )
    async def checkkey(interaction: discord.Interaction, key: str):
        """
        Check the status of a license key.
        
        Args:
            interaction: The Discord interaction
            key: The license key to check
        """
        # Always make this response ephemeral (private) to protect key info
        await interaction.response.defer(ephemeral=True)
        
        try:
            # Get all keys
            keys_response = await api.get_all_keys()
            all_keys = keys_response.get("keys", [])
            
            # Find the matching key
            matching_key = next((k for k in all_keys if k.get("value") == key), None)
            
            if not matching_key:
                await interaction.followup.send(
                    "This key does not exist in our system.",
                    ephemeral=True
                )
                return
            
            # Get key information
            key_type = matching_key.get("typeName", "Unknown")
            is_used = matching_key.get("used", False)
            discord_username = matching_key.get("discordUsername", "None")
            
            # Create response embed
            embed = discord.Embed(
                title="License Key Information",
                color=discord.Color.blue()
            )
            embed.add_field(name="Key", value=f"`{key}`", inline=False)
            embed.add_field(name="Type", value=key_type, inline=True)
            embed.add_field(name="Status", value="Used" if is_used else "Available", inline=True)
            
            if is_used:
                embed.add_field(name="Assigned To", value=discord_username, inline=True)
                
                # Check if this key belongs to the user
                if discord_username == interaction.user.name:
                    embed.color = discord.Color.green()
                    embed.set_footer(text="This key is registered to you.")
                else:
                    embed.color = discord.Color.red()
                    embed.set_footer(text="This key is registered to another user.")
            
            await interaction.followup.send(embed=embed, ephemeral=True)
        except Exception as e:
            logger.error(f"Error in checkkey command: {str(e)}")
            await interaction.followup.send(
                "There was an error checking this key. Please try again later.",
                ephemeral=True
            )

    @bot.tree.command(
        name="keyavailability",
        description="Check the availability of license keys"
    )
    async def keyavailability(interaction: discord.Interaction):
        """
        Check the availability of different types of license keys.
        
        Args:
            interaction: The Discord interaction
        """
        await interaction.response.defer()
        
        try:
            # Get key statistics
            stats = await api.get_stats()
            
            # Create embed
            embed = discord.Embed(
                title="Key Availability",
                description="Current availability of license keys",
                color=discord.Color.blue()
            )
            
            # Add overall statistics
            embed.add_field(
                name="Overall",
                value=f"Available: {stats.get('availableKeys', 0)} / {stats.get('totalKeys', 0)}",
                inline=False
            )
            
            # Add type-specific statistics
            key_types = stats.get("keysByType", {})
            for type_name, type_stats in key_types.items():
                available = type_stats.get("available", 0)
                total = type_stats.get("total", 0)
                
                # Set color based on availability
                if total == 0:
                    status_emoji = "⚠️"  # Warning emoji for no keys
                elif available == 0:
                    status_emoji = "🔴"  # Red circle for no available keys
                elif available / total < 0.2:
                    status_emoji = "🟠"  # Orange circle for low availability
                else:
                    status_emoji = "🟢"  # Green circle for good availability
                
                embed.add_field(
                    name=f"{status_emoji} {type_name}",
                    value=f"Available: {available} / {total}",
                    inline=True
                )
            
            await interaction.followup.send(embed=embed)
        except Exception as e:
            logger.error(f"Error in keyavailability command: {str(e)}")
            await interaction.followup.send(
                "There was an error retrieving key availability. Please try again later."
            )