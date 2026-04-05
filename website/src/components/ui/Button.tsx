import type { ReactNode } from 'react';
import { Link } from 'react-router-dom';

interface ButtonProps {
  variant?: 'primary' | 'secondary' | 'ghost';
  href?: string;
  onClick?: () => void;
  children: ReactNode;
  className?: string;
}

export function Button({ variant = 'primary', href, onClick, children, className = '' }: ButtonProps) {
  const base: React.CSSProperties = {
    display: 'inline-flex',
    alignItems: 'center',
    gap: '0.5rem',
    padding: '0.6rem 1.4rem',
    fontSize: '0.8rem',
    fontWeight: 600,
    borderRadius: '6px',
    cursor: 'pointer',
    textDecoration: 'none',
    transition: 'all 0.2s ease',
    border: '1px solid transparent',
    letterSpacing: '0.01em',
  };

  const variants: Record<string, React.CSSProperties> = {
    primary: {
      ...base,
      backgroundColor: 'var(--color-accent)',
      color: '#0c0e12',
    },
    secondary: {
      ...base,
      backgroundColor: 'transparent',
      color: 'var(--color-text-primary)',
      border: '1px solid var(--color-border-subtle)',
    },
    ghost: {
      ...base,
      backgroundColor: 'transparent',
      color: 'var(--color-text-secondary)',
    },
  };

  const style = variants[variant] ?? variants.primary;

  if (href) {
    if (href.startsWith('mailto:') || href.startsWith('http')) {
      return (
        <a href={href} style={style} className={className}>
          {children}
        </a>
      );
    }
    return (
      <Link to={href} style={style} className={className}>
        {children}
      </Link>
    );
  }

  return (
    <button type="button" onClick={onClick} style={style} className={className}>
      {children}
    </button>
  );
}
