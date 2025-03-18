"""
Logging configuration for the Key Management System Discord Bot.
"""
import logging
import sys
import os
from datetime import datetime
from pathlib import Path

def setup_logger() -> logging.Logger:
    """
    Set up and configure the logger.
    
    Returns:
        logging.Logger: Configured logger instance
    """
    # Get the root logger
    logger = logging.getLogger()
    
    # Set log level from environment variable or use INFO as default
    log_level_name = os.getenv("LOG_LEVEL", "INFO").upper()
    log_level = getattr(logging, log_level_name, logging.INFO)
    logger.setLevel(log_level)
    
    # Create logs directory if it doesn't exist
    logs_dir = Path("logs")
    logs_dir.mkdir(exist_ok=True)
    
    # Create a formatter
    formatter = logging.Formatter(
        "%(asctime)s - %(name)s - %(levelname)s - %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S"
    )
    
    # Create console handler
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)
    
    # Create file handler with date in filename
    current_date = datetime.now().strftime("%Y-%m-%d")
    file_handler = logging.FileHandler(
        logs_dir / f"{current_date}.log",
        encoding="utf-8"
    )
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    
    return logger