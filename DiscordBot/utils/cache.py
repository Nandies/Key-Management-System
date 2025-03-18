"""
Caching utilities for the Key Management System Discord Bot.
"""
import sqlite3
import json
import logging
import asyncio
from datetime import datetime, timedelta
from typing import Dict, Any, Optional, Union
from pathlib import Path

logger = logging.getLogger(__name__)

# Initialize the cache database
DB_PATH = Path("cache.db")

# Database connection lock to handle concurrent access
_db_lock = asyncio.Lock()

async def _init_db() -> None:
    """Initialize the cache database if it doesn't exist."""
    async with _db_lock:
        try:
            conn = sqlite3.connect(str(DB_PATH))
            cursor = conn.cursor()
            
            # Create the cache table
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS cache (
                key TEXT PRIMARY KEY,
                value TEXT NOT NULL,
                created_at TIMESTAMP NOT NULL
            )
            """)
            
            conn.commit()
            conn.close()
        except Exception as e:
            logger.error(f"Error initializing cache database: {str(e)}")

async def get_cache(
    key: str,
    enabled: bool = True,
    expiry_minutes: int = 5
) -> Optional[Dict[str, Any]]:
    """
    Get an item from the cache.
    
    Args:
        key: Cache key
        enabled: Whether caching is enabled
        expiry_minutes: Cache expiry time in minutes
        
    Returns:
        Optional[Dict[str, Any]]: Cached value or None if not found or expired
    """
    if not enabled:
        return None
    
    await _init_db()
    
    async with _db_lock:
        try:
            conn = sqlite3.connect(str(DB_PATH))
            cursor = conn.cursor()
            
            # Get the cache item
            cursor.execute(
                "SELECT value, created_at FROM cache WHERE key = ?",
                (key,)
            )
            
            result = cursor.fetchone()
            conn.close()
            
            if not result:
                return None
            
            value_str, created_at_str = result
            
            # Check if the cache item has expired
            created_at = datetime.fromisoformat(created_at_str)
            now = datetime.now()
            
            if now - created_at > timedelta(minutes=expiry_minutes):
                # Cache has expired
                await delete_cache(key)
                return None
            
            # Parse the JSON value
            return json.loads(value_str)
        except Exception as e:
            logger.error(f"Error getting cache item: {str(e)}")
            return None

async def update_cache(key: str, value: Union[Dict[str, Any], list, str, int, float, bool]) -> bool:
    """
    Update or create a cache item.
    
    Args:
        key: Cache key
        value: Value to cache
        
    Returns:
        bool: True if successful, False otherwise
    """
    await _init_db()
    
    async with _db_lock:
        try:
            conn = sqlite3.connect(str(DB_PATH))
            cursor = conn.cursor()
            
            # Convert value to JSON string
            if isinstance(value, (dict, list)) or isinstance(value, (str, int, float, bool)):
                value_str = json.dumps(value)
            else:
                logger.error(f"Cannot cache value of type {type(value)}")
                conn.close()
                return False
            
            # Current timestamp
            now = datetime.now().isoformat()
            
            # Insert or replace the cache item
            cursor.execute(
                """
                INSERT OR REPLACE INTO cache (key, value, created_at)
                VALUES (?, ?, ?)
                """,
                (key, value_str, now)
            )
            
            conn.commit()
            conn.close()
            
            return True
        except Exception as e:
            logger.error(f"Error updating cache: {str(e)}")
            return False

async def delete_cache(key: str) -> bool:
    """
    Delete a cache item.
    
    Args:
        key: Cache key
        
    Returns:
        bool: True if successful, False otherwise
    """
    await _init_db()
    
    async with _db_lock:
        try:
            conn = sqlite3.connect(str(DB_PATH))
            cursor = conn.cursor()
            
            # Delete the cache item
            cursor.execute("DELETE FROM cache WHERE key = ?", (key,))
            
            conn.commit()
            conn.close()
            
            return True
        except Exception as e:
            logger.error(f"Error deleting cache item: {str(e)}")
            return False

async def clear_cache() -> bool:
    """
    Clear all cache items.
    
    Returns:
        bool: True if successful, False otherwise
    """
    await _init_db()
    
    async with _db_lock:
        try:
            conn = sqlite3.connect(str(DB_PATH))
            cursor = conn.cursor()
            
            # Delete all cache items
            cursor.execute("DELETE FROM cache")
            
            conn.commit()
            conn.close()
            
            return True
        except Exception as e:
            logger.error(f"Error clearing cache: {str(e)}")
            return False

async def cleanup_expired_cache(expiry_minutes: int = 60) -> int:
    """
    Remove expired cache entries.
    
    Args:
        expiry_minutes: Time in minutes after which entries are considered expired
        
    Returns:
        int: Number of removed entries or -1 on error
    """
    await _init_db()
    
    async with _db_lock:
        try:
            conn = sqlite3.connect(str(DB_PATH))
            cursor = conn.cursor()
            
            # Calculate expiry timestamp
            expiry_time = (datetime.now() - timedelta(minutes=expiry_minutes)).isoformat()
            
            # Delete expired cache items
            cursor.execute(
                "DELETE FROM cache WHERE created_at < ?",
                (expiry_time,)
            )
            
            removed_count = cursor.rowcount
            conn.commit()
            conn.close()
            
            if removed_count > 0:
                logger.info(f"Removed {removed_count} expired cache entries")
            
            return removed_count
        except Exception as e:
            logger.error(f"Error cleaning up expired cache: {str(e)}")
            return -1

async def vacuum_database() -> bool:
    """
    Optimize the cache database by running VACUUM.
    
    Returns:
        bool: True if successful, False otherwise
    """
    await _init_db()
    
    async with _db_lock:
        try:
            conn = sqlite3.connect(str(DB_PATH))
            cursor = conn.cursor()
            
            # Get database size before vacuum
            cursor.execute("PRAGMA page_count")
            before_pages = cursor.fetchone()[0]
            cursor.execute("PRAGMA page_size")
            page_size = cursor.fetchone()[0]
            before_size = before_pages * page_size / 1024  # Size in KB
            
            # Run VACUUM to optimize the database
            cursor.execute("VACUUM")
            
            # Get database size after vacuum
            cursor.execute("PRAGMA page_count")
            after_pages = cursor.fetchone()[0]
            after_size = after_pages * page_size / 1024  # Size in KB
            
            conn.commit()
            conn.close()
            
            savings = before_size - after_size
            logger.info(f"Database optimized: {before_size:.2f}KB -> {after_size:.2f}KB (saved {savings:.2f}KB)")
            
            return True
        except Exception as e:
            logger.error(f"Error optimizing database: {str(e)}")
            return False