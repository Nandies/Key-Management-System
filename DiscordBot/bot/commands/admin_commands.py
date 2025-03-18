"""
Admin-only commands for the Discord bot.
"""
import discord
from discord import app_commands
from discord.ext import commands
import logging
from api.client import KeyManagementAPI
from utils.webhook import send_admin_notification
from datetime import datetime

logger = logging.getLogger(__name__)

# Check if user has the Key Admin role
def is_key_admin():
    def predicate(interaction: discord.Interaction) -> bool:
        return any(role.name.lower() == "key admin" for role in interaction.user.roles)
    return app_commands.check(predicate)

async def setup(bot: commands.Bot) -> None:
    """
    Set up admin commands.
    
    Args:
        bot: The Discord bot instance
    """
    # Create API client
    api = KeyManagementAPI(bot.config.api_base_url, bot.config.api_key)
    
    # Create admin command group
    admin_group = app_commands.Group(
        name="admin",
        description="Key management administrative commands",
        guild_only=True
    )
    
    @admin_group.command(name="stats", description="Get detailed key statistics")
    @is_key_admin()
    async def key_stats(interaction: discord.Interaction):
        """
        Get detailed key statistics.
        
        Args:
            interaction: The Discord interaction
        """
        await interaction.response.defer()
        
        try:
            # Get key statistics
            stats = await api.get_stats()
            
            # Create embed
            embed = discord.Embed(
                title="Key Statistics",
                description="Detailed statistics for license keys",
                color=discord.Color.blue(),
                timestamp=datetime.now()
            )
            
            # Add overall statistics
            total_keys = stats.get("totalKeys", 0)
            used_keys = stats.get("usedKeys", 0)
            available_keys = stats.get("availableKeys", 0)
            
            embed.add_field(name="Total Keys", value=str(total_keys), inline=True)
            embed.add_field(name="Used Keys", value=str(used_keys), inline=True)
            embed.add_field(name="Available Keys", value=str(available_keys), inline=True)
            
            # Add type-specific statistics
            key_types = stats.get("keysByType", {})
            
            for type_name, type_stats in key_types.items():
                total = type_stats.get("total", 0)
                used = type_stats.get("used", 0)
                available = type_stats.get("available", 0)
                
                if total > 0:
                    usage_percent = (used / total) * 100
                    embed.add_field(
                        name=f"{type_name} Keys",
                        value=f"Total: {total}\nUsed: {used} ({usage_percent:.1f}%)\nAvailable: {available}",
                        inline=True
                    )
            
            # Add usage information
            if total_keys > 0:
                total_usage_percent = (used_keys / total_keys) * 100
                embed.set_footer(text=f"Overall Usage: {total_usage_percent:.1f}%")
            
            await interaction.followup.send(embed=embed)
        except Exception as e:
            logger.error(f"Error in key_stats command: {str(e)}")
            await interaction.followup.send(
                "There was an error retrieving key statistics. Please try again later."
            )
    
    @admin_group.command(name="listkeys", description="List keys of a specific type")
    @app_commands.describe(
        key_type="Type of keys to list",
        show_used="Whether to include used keys in the listing",
        limit="Maximum number of keys to show (default: 10)"
    )
    @app_commands.choices(key_type=[
        app_commands.Choice(name="Daily", value="0"),
        app_commands.Choice(name="Weekly", value="1"),
        app_commands.Choice(name="Monthly", value="2"),
        app_commands.Choice(name="Lifetime", value="3")
    ])
    @is_key_admin()
    async def list_keys(
        interaction: discord.Interaction,
        key_type: app_commands.Choice[str],
        show_used: bool = False,
        limit: int = 10
    ):
        """
        List keys of a specific type.
        
        Args:
            interaction: The Discord interaction
            key_type: Type of keys to list
            show_used: Whether to include used keys in the listing
            limit: Maximum number of keys to show
        """
        await interaction.response.defer(ephemeral=True)
        
        try:
            # Get keys by type
            keys_response = await api.get_keys_by_type(key_type.value)
            
            all_keys = keys_response.get("keys", [])
            
            # Filter keys
            if not show_used:
                all_keys = [k for k in all_keys if not k.get("used")]
            
            # Limit number of keys shown
            if limit > 25:
                limit = 25  # Cap at 25 to prevent oversized messages
                
            keys_to_show = all_keys[:limit]
            
            if not keys_to_show:
                if show_used:
                    message = f"No {key_type.name} keys found."
                else:
                    message = f"No available {key_type.name} keys found."
                await interaction.followup.send(message, ephemeral=True)
                return
            
            # Create embed
            embed = discord.Embed(
                title=f"{key_type.name} Keys",
                description=f"Showing {len(keys_to_show)} out of {len(all_keys)} keys",
                color=discord.Color.blue()
            )
            
            # Add keys to embed
            for key in keys_to_show:
                key_value = key.get("value", "N/A")
                key_status = "Used" if key.get("used") else "Available"
                key_username = key.get("discordUsername", "N/A") if key.get("used") else "N/A"
                
                embed.add_field(
                    name=f"Key: {key_value}",
                    value=f"Status: {key_status}\nUser: {key_username}",
                    inline=False
                )
            
            await interaction.followup.send(embed=embed, ephemeral=True)
        except Exception as e:
            logger.error(f"Error in list_keys command: {str(e)}")
            await interaction.followup.send(
                "There was an error listing keys. Please try again later.",
                ephemeral=True
            )
    
    @admin_group.command(name="addkey", description="Add a new license key")
    @app_commands.describe(
        key_value="The license key to add",
        key_type="Type of key to add"
    )
    @app_commands.choices(key_type=[
        app_commands.Choice(name="Daily", value="0"),
        app_commands.Choice(name="Weekly", value="1"),
        app_commands.Choice(name="Monthly", value="2"),
        app_commands.Choice(name="Lifetime", value="3")
    ])
    @is_key_admin()
    async def add_key(
        interaction: discord.Interaction,
        key_value: str,
        key_type: app_commands.Choice[str]
    ):
        """
        Add a new license key.
        
        Args:
            interaction: The Discord interaction
            key_value: The license key to add
            key_type: Type of key to add
        """
        # Validate key format
        if not key_value or len(key_value) < 5 or len(key_value) > 50:
            await interaction.response.send_message(
                "Invalid key format. Keys must be between 5 and 50 characters in length.",
                ephemeral=True
            )
            return
            
        # Basic validation - check for common patterns or formats if needed
        # For example, if keys should match a specific pattern like XXX-XXX-XXX
        # import re
        # if not re.match(r'^[A-Z0-9]{3}-[A-Z0-9]{3}-[A-Z0-9]{3}
        """
        Add a new license key.
        
        Args:
            interaction: The Discord interaction
            key_value: The license key to add
            key_type: Type of key to add
        """
        await interaction.response.defer(ephemeral=True)
        
        try:
            # Add the key
            success = await api.add_key(key_value, key_type.value)
            
            if success:
                await interaction.followup.send(
                    f"Successfully added {key_type.name} key: `{key_value}`",
                    ephemeral=True
                )
                
                # Log key addition
                logger.info(f"Key {key_value} ({key_type.name}) added by {interaction.user.name}")
                
                # Send notification via webhook if configured
                if bot.config.admin_webhook_url:
                    await send_admin_notification(
                        webhook_url=bot.config.admin_webhook_url,
                        title="Key Added",
                        description=f"A {key_type.name} key was added by {interaction.user.name}",
                        color=0x00FF00  # Green
                    )
            else:
                await interaction.followup.send(
                    f"Failed to add key. The key may already exist.",
                    ephemeral=True
                )
        except Exception as e:
            logger.error(f"Error in add_key command: {str(e)}")
            await interaction.followup.send(
                "There was an error adding the key. Please try again later.",
                ephemeral=True
            )
    
    @admin_group.command(name="unassignkey", description="Unassign a key from a user")
    @app_commands.describe(
        key_value="The license key to unassign"
    )
    @is_key_admin()
    async def unassign_key(interaction: discord.Interaction, key_value: str):
        """
        Unassign a key from a user.
        
        Args:
            interaction: The Discord interaction
            key_value: The license key to unassign
        """
        await interaction.response.defer(ephemeral=True)
        
        try:
            # Get all keys
            keys_response = await api.get_all_keys()
            all_keys = keys_response.get("keys", [])
            
            # Find the key by value
            key = next((k for k in all_keys if k.get("value") == key_value), None)
            
            if not key:
                await interaction.followup.send(
                    "Key not found. Please check the key value and try again.",
                    ephemeral=True
                )
                return
            
            key_id = key.get("id")
            is_used = key.get("used", False)
            username = key.get("discordUsername", "")
            
            if not is_used:
                await interaction.followup.send(
                    "This key is not currently assigned to any user.",
                    ephemeral=True
                )
                return
            
            # Unassign the key
            success = await api.mark_key_as_unused(key_id)
            
            if success:
                await interaction.followup.send(
                    f"Successfully unassigned key `{key_value}` from {username}",
                    ephemeral=True
                )
                
                # Log key unassignment
                logger.info(f"Key {key_value} unassigned from {username} by {interaction.user.name}")
                
                # Send notification via webhook if configured
                if bot.config.admin_webhook_url:
                    await send_admin_notification(
                        webhook_url=bot.config.admin_webhook_url,
                        title="Key Unassigned",
                        description=f"Key: `{key_value}`\nPrevious User: {username}\nUnassigned by: {interaction.user.name}",
                        color=0xFFA500  # Orange
                    )
            else:
                await interaction.followup.send(
                    "Failed to unassign key. Please try again later.",
                    ephemeral=True
                )
        except Exception as e:
            logger.error(f"Error in unassign_key command: {str(e)}")
            await interaction.followup.send(
                "There was an error unassigning the key. Please try again later.",
                ephemeral=True
            )
    
    # Register the admin command group with the bot
    bot.tree.add_command(admin_group)
, key_value):
        #     await interaction.response.send_message("Invalid key format.", ephemeral=True)
        #     return
        """
        Add a new license key.
        
        Args:
            interaction: The Discord interaction
            key_value: The license key to add
            key_type: Type of key to add
        """
        await interaction.response.defer(ephemeral=True)
        
        try:
            # Add the key
            success = await api.add_key(key_value, key_type.value)
            
            if success:
                await interaction.followup.send(
                    f"Successfully added {key_type.name} key: `{key_value}`",
                    ephemeral=True
                )
                
                # Log key addition
                logger.info(f"Key {key_value} ({key_type.name}) added by {interaction.user.name}")
                
                # Send notification via webhook if configured
                if bot.config.admin_webhook_url:
                    await send_admin_notification(
                        webhook_url=bot.config.admin_webhook_url,
                        title="Key Added",
                        description=f"A {key_type.name} key was added by {interaction.user.name}",
                        color=0x00FF00  # Green
                    )
            else:
                await interaction.followup.send(
                    f"Failed to add key. The key may already exist.",
                    ephemeral=True
                )
        except Exception as e:
            logger.error(f"Error in add_key command: {str(e)}")
            await interaction.followup.send(
                "There was an error adding the key. Please try again later.",
                ephemeral=True
            )
    
    @admin_group.command(name="unassignkey", description="Unassign a key from a user")
    @app_commands.describe(
        key_value="The license key to unassign"
    )
    @is_key_admin()
    async def unassign_key(interaction: discord.Interaction, key_value: str):
        """
        Unassign a key from a user.
        
        Args:
            interaction: The Discord interaction
            key_value: The license key to unassign
        """
        await interaction.response.defer(ephemeral=True)
        
        try:
            # Get all keys
            keys_response = await api.get_all_keys()
            all_keys = keys_response.get("keys", [])
            
            # Find the key by value
            key = next((k for k in all_keys if k.get("value") == key_value), None)
            
            if not key:
                await interaction.followup.send(
                    "Key not found. Please check the key value and try again.",
                    ephemeral=True
                )
                return
            
            key_id = key.get("id")
            is_used = key.get("used", False)
            username = key.get("discordUsername", "")
            
            if not is_used:
                await interaction.followup.send(
                    "This key is not currently assigned to any user.",
                    ephemeral=True
                )
                return
            
            # Unassign the key
            success = await api.mark_key_as_unused(key_id)
            
            if success:
                await interaction.followup.send(
                    f"Successfully unassigned key `{key_value}` from {username}",
                    ephemeral=True
                )
                
                # Log key unassignment
                logger.info(f"Key {key_value} unassigned from {username} by {interaction.user.name}")
                
                # Send notification via webhook if configured
                if bot.config.admin_webhook_url:
                    await send_admin_notification(
                        webhook_url=bot.config.admin_webhook_url,
                        title="Key Unassigned",
                        description=f"Key: `{key_value}`\nPrevious User: {username}\nUnassigned by: {interaction.user.name}",
                        color=0xFFA500  # Orange
                    )
            else:
                await interaction.followup.send(
                    "Failed to unassign key. Please try again later.",
                    ephemeral=True
                )
        except Exception as e:
            logger.error(f"Error in unassign_key command: {str(e)}")
            await interaction.followup.send(
                "There was an error unassigning the key. Please try again later.",
                ephemeral=True
            )
    
    # Register the admin command group with the bot
    bot.tree.add_command(admin_group)