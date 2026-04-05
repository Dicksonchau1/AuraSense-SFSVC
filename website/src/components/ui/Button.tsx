import { Link } from 'react-router-dom';
import type { ReactNode, MouseEventHandler } from 'react';

interface ButtonProps {
  variant?: 'primary' | 'secondary' | 'ghost';
  size?: 'sm' | 'md' | 'lg';
  href?: string;
  children: ReactNode;
  className?: string;
  onClick?: MouseEventHandler<HTMLButtonElement>;
}

const variantStyles: Record<string, string> = {
  primary:
    'bg-accent text-surface hover:bg-accent-muted focus-visible:ring-accent',
  secondary:
    'border border-border-subtle text-text-primary hover:border-accent focus-visible:ring-accent',
  ghost:
    'text-text-secondary hover:text-accent focus-visible:ring-accent',
};

const sizeStyles: Record<string, string> = {
  sm: 'px-3 py-1.5 text-sm',
  md: 'px-5 py-2.5 text-sm',
  lg: 'px-7 py-3 text-base',
};

export function Button({
  variant = 'primary',
  size = 'md',
  href,
  children,
  className = '',
  onClick,
}: ButtonProps) {
  const base =
    'inline-flex items-center justify-center font-medium rounded-md transition-colors duration-150 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-offset-2 focus-visible:ring-offset-surface';
  const classes = `${base} ${variantStyles[variant]} ${sizeStyles[size]} ${className}`.trim();

  if (href) {
    return (
      <Link to={href} className={classes}>
        {children}
      </Link>
    );
  }

  return (
    <button type="button" className={classes} onClick={onClick}>
      {children}
    </button>
  );
}
